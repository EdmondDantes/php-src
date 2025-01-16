/*
+----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Edmond                                                       |
  +----------------------------------------------------------------------+
*/
#include "curl_async.h"
#include "Zend/zend_types.h"
#include <zend_exceptions.h>
#include <async/php_reactor.h>
#include <async/php_layer/callback.h>
#include <async/php_layer/zend_common.h>

/********************************************************************************************************************
 * This module is designed for the asynchronous version of PHP functions that work with CURL.
 * The module implements two entry points:
 * * `curl_async_perform`
 * * `curl_async_wait`
 *
 * which replace the calls to the corresponding CURL LIB functions.
 *
 * `curl_async_perform` internally uses a global CURL MULTI object to execute multiple CURL requests asynchronously.
 * The handlers `CURLMOPT_SOCKETFUNCTION` and `CURLMOPT_TIMERFUNCTION` ensure the integration of
 * CURL with the `PHP ASYNC Reactor` (Event Loop).
 *
 * Note that the algorithm for working with the Resume object is modified here.
 * Typically, all descriptors associated with a Resume object are declared before the Fiber is suspended.
 * However, in the case of CURL, descriptors are declared and added to the Resume object after the Fiber is suspended.
 * This is due to the integration logic with CURL.
 *
 * Keep in mind that if the Resume object is destroyed,
 * all associated descriptors will also be destroyed and removed from the event loop.
 *
 * `curl_async_wait` is a wrapper for the CURL function `curl_multi_wait`.
 * It suspends the execution of the Fiber until the first resolved descriptor.
 * It operates in a manner similar to the previous function,
 * with the difference that `curl_async_wait` creates a special `Resume` object
 * with additional context that participates in callbacks `curl_async_context`.
 *
 * ******************************************************************************************************************
 */

ZEND_TLS CURLM * curl_multi_handle = NULL;
ZEND_TLS HashTable * curl_multi_resume_list = NULL;
ZEND_TLS reactor_handle_t * timer = NULL;
ZEND_TLS zend_object * poll_callback_obj = NULL;
ZEND_TLS zend_object * timer_callback_obj = NULL;

static void process_curl_completed_handles(void)
{
	CURLMsg *msg;
	int msgs_in_queue = 0;

	while ((msg = curl_multi_info_read(curl_multi_handle, &msgs_in_queue))) {
		if (msg->msg == CURLMSG_DONE) {

			curl_multi_remove_handle(curl_multi_handle, msg->easy_handle);

			const zval * resume = zend_hash_index_find(curl_multi_resume_list, (zend_ulong) msg->easy_handle);

			if (resume == NULL) {
				continue;
			}

			zval result;
			ZVAL_LONG(&result, msg->data.result);
			async_resume_fiber((async_resume_t *) Z_OBJ_P(resume), &result, NULL);
		}
	}
}

static void poll_callback(zend_object * callback, zend_object * notifier, const zval* z_event, const zval* error)
{
	const reactor_poll_t * handle = (reactor_poll_t *) notifier;
	const zend_long events = Z_LVAL_P(z_event);
	int action = 0;

	if (events & ASYNC_READABLE) {
		action |= CURL_CSELECT_IN;
	}

	if (events & ASYNC_WRITABLE) {
		action |= CURL_CSELECT_OUT;
	}

	if (Z_TYPE_P(error) == IS_OBJECT) {
		action |= CURL_CSELECT_ERR;
	}

	curl_multi_socket_action(curl_multi_handle, handle->socket, action, NULL);

	process_curl_completed_handles();
}

static void curl_poll_callback(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error)
{
	const reactor_poll_t * handle = (reactor_poll_t *) notifier;
	const zend_long events = Z_LVAL_P(event);
	int action = 0;

	if (events & ASYNC_READABLE) {
		action |= CURL_CSELECT_IN;
	}

	if (events & ASYNC_WRITABLE) {
		action |= CURL_CSELECT_OUT;
	}

	if (Z_TYPE_P(error) == IS_OBJECT) {
		action |= CURL_CSELECT_ERR;
	}

	curl_multi_socket_action(curl_multi_handle, handle->socket, action, NULL);
	process_curl_completed_handles();
}


static int curl_socket_cb(CURL *curl, const curl_socket_t socket_fd, const int what, void *user_p, void *socket_poll)
{
	const zval * resume = zend_hash_index_find(curl_multi_resume_list, (zend_ulong) curl);

	if (resume == NULL) {
		return 0;
	}

	if (what == CURL_POLL_REMOVE) {

		if (socket_poll == NULL) {
			return 0;
		}

		reactor_remove_handle_fn((reactor_handle_t *) socket_poll);
		async_resume_remove_notifier((async_resume_t *) Z_OBJ_P(resume), socket_poll);
		return 0;
	}

	if (socket_poll == NULL) {

		zend_long events = 0;

		if (what & CURL_POLL_IN) {
			events |= ASYNC_READABLE;
		}

		if (what & CURL_POLL_OUT) {
			events |= ASYNC_WRITABLE;
		}

		socket_poll = reactor_socket_new_fn((php_socket_t) socket_fd, events);

		if (socket_poll == NULL) {
			return CURLM_BAD_SOCKET;
		}

		async_resume_when((async_resume_t *) Z_OBJ_P(resume), socket_poll, true, curl_poll_callback);

		if (EG(exception)) {
			OBJ_RELEASE(&((reactor_handle_t *) socket_poll)->std);
			zend_exception_to_warning("Failed to add poll callback: %s", true);
			return CURLM_BAD_SOCKET;
		}

		curl_multi_assign(curl_multi_handle, socket_fd, socket_poll);

		reactor_add_handle(socket_poll);
	}

	return 0;
}

static void timer_callback(zend_object * callback, zend_object *notifier, const zval* z_event, const zval* error)
{
	curl_multi_socket_action(curl_multi_handle, CURL_SOCKET_TIMEOUT, 0, NULL);
	process_curl_completed_handles();
}

static int curl_timer_cb(CURLM *multi, const long timeout_ms, void *user_p)
{
	if (timeout_ms < 0) {
		if (timer != NULL) {
			reactor_remove_handle_fn(timer);
			OBJ_RELEASE(&timer->std);
		}

		return 0;
	}

	if (timer != NULL) {
		reactor_remove_handle_fn(timer);
		OBJ_RELEASE(&timer->std);
	}

	timer = reactor_timer_new_fn(timeout_ms, false);

	if (timer == NULL) {
		return CURLM_INTERNAL_ERROR;
	}

	if (timer_callback_obj == NULL) {
		timer_callback_obj = async_callback_new(timer_callback);

		if (timer_callback_obj == NULL) {
            OBJ_RELEASE(&timer->std);
            return CURLM_INTERNAL_ERROR;
        }
	}

	zval z_timer_callback;
	ZVAL_OBJ(&z_timer_callback, timer_callback_obj);
	async_notifier_add_callback(&timer->std, &z_timer_callback);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&timer->std);
		zend_exception_to_warning("Failed to add timer callback: %s", true);
		return CURLM_INTERNAL_ERROR;
	}

	reactor_add_handle(timer);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&timer->std);
		zend_exception_to_warning("Failed to add timer handle: %s", true);
		return CURLM_INTERNAL_ERROR;
	}

	return 0;
}

void curl_async_setup(void)
{
	if (curl_multi_handle != NULL) {
		return;
	}

	curl_multi_handle = curl_multi_init();
	curl_multi_setopt(curl_multi_handle, CURLMOPT_SOCKETFUNCTION, curl_socket_cb);
	curl_multi_setopt(curl_multi_handle, CURLMOPT_TIMERFUNCTION, curl_timer_cb);
	curl_multi_setopt(curl_multi_handle, CURLMOPT_SOCKETDATA, NULL);
	curl_multi_resume_list = zend_new_array(8);

	timer = NULL;
	poll_callback_obj = NULL;
	timer_callback_obj = NULL;
}

void curl_async_shutdown(void)
{
	if (timer != NULL) {
        reactor_remove_handle_fn(timer);
        OBJ_RELEASE(&timer->std);
        timer = NULL;
    }

	if (poll_callback_obj != NULL) {
        OBJ_RELEASE(poll_callback_obj);
        poll_callback_obj = NULL;
    }

	if (timer_callback_obj != NULL) {
        OBJ_RELEASE(timer_callback_obj);
        timer_callback_obj = NULL;
    }

	if (curl_multi_handle != NULL) {
		curl_multi_cleanup(curl_multi_handle);
		curl_multi_handle = NULL;
	}

	if (curl_multi_resume_list != NULL) {
		zend_array_destroy(curl_multi_resume_list);
		curl_multi_resume_list = NULL;
	}
}

//=============================================================
CURLcode curl_async_perform(CURL* curl)
{
	if (curl_multi_handle == NULL) {
		curl_async_setup();
	}

	// Add curl handle to curl_multi_resume_list
	zval z_resume;
	async_resume_t * resume = async_resume_new(NULL);

	if (resume == NULL) {
		return CURLE_FAILED_INIT;
	}

	ZVAL_OBJ(&z_resume, &resume->std);

	// add z_resume to curl_multi_resume_list
	if (zend_hash_index_update(curl_multi_resume_list, (zend_ulong) curl, &z_resume) == NULL) {
		zval_ptr_dtor(&z_resume);
		return CURLE_FAILED_INIT;
	}

	curl_multi_add_handle(curl_multi_handle, curl);
	curl_multi_socket_action(curl_multi_handle, CURL_SOCKET_TIMEOUT, 0, NULL);

	async_await(resume);

	ZEND_ASSERT(GC_REFCOUNT(&resume->std) == 1 && "Memory leak detected. The resume object should have only one reference");
	zend_hash_index_del(curl_multi_resume_list, (zend_ulong) curl);
	OBJ_RELEASE(&resume->std);

	return CURLE_OK;
}

//=============================================================
#pragma region curl_async_wait
//=============================================================

static void multi_timer_callback(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error)
{
	curl_multi_socket_action(((curl_async_context *) resume)->curl_multi_handle, CURL_SOCKET_TIMEOUT, 0, NULL);
}

static int multi_timer_cb(CURLM *multi, const long timeout_ms, void *user_p)
{
	curl_async_context * context = user_p;

	if (context == NULL) {
        return CURLM_INTERNAL_ERROR;
    }

	if (timeout_ms < 0) {
		if (context->timer != NULL) {
			async_resume_remove_notifier(&context->resume, context->timer);
		}

		return 0;
	}

	if (timer != NULL) {
		async_resume_remove_notifier(&context->resume, context->timer);
	}

	context->timer = reactor_timer_new_fn(timeout_ms, false);

	if (context->timer == NULL) {
		return CURLM_INTERNAL_ERROR;
	}

	async_resume_when(&context->resume, context->timer, true, multi_timer_callback);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&context->timer->std);
		context->timer = NULL;
		return CURLM_INTERNAL_ERROR;
	}

	reactor_add_handle(context->timer);

	if (UNEXPECTED(EG(exception))) {
		async_resume_remove_notifier(&context->resume, context->timer);
		zend_exception_to_warning("Failed to add timer handle: %s", true);
		return CURLM_INTERNAL_ERROR;
	}

	return 0;
}

static void multi_poll_callback(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error)
{
	curl_async_context * context = (curl_async_context *) resume;

	const reactor_poll_t * handle = (reactor_poll_t *) notifier;
	const zend_long events = Z_LVAL_P(event);
	int action = 0;

	if (events & ASYNC_READABLE) {
		action |= CURL_CSELECT_IN;
	}

	if (events & ASYNC_WRITABLE) {
		action |= CURL_CSELECT_OUT;
	}

	if (Z_TYPE_P(error) == IS_OBJECT) {
		action |= CURL_CSELECT_ERR;
	}

	curl_multi_socket_action(context->curl_multi_handle, handle->socket, action, NULL);
	async_resume_when_callback_resolve(resume, notifier, event, error);
}

static int multi_socket_cb(CURL *curl, const curl_socket_t socket_fd, const int what, void *user_p, void *data)
{
	curl_async_context * context = user_p;

	if (context == NULL) {
        return -1;
    }

	if (what == CURL_POLL_REMOVE) {

		if (context->poll_list == NULL) {
			return 0;
		}

		const zval * z_handle = zend_hash_index_find(context->poll_list, socket_fd);

		if (z_handle == NULL) {
            return 0;
        }

		reactor_handle_t * handle = (reactor_handle_t *) Z_OBJ_P(z_handle);

		zend_hash_index_del(context->poll_list, socket_fd);
		reactor_remove_handle_fn(handle);
		async_resume_remove_notifier(&context->resume, handle);

		return 0;
	}

	if (context->poll_list == NULL) {
        context->poll_list = emalloc(sizeof(HashTable));
		zend_hash_init(context->poll_list, 4, NULL, NULL, false);
    }

	zend_long events = 0;

	if (what & CURL_POLL_IN) {
		events |= ASYNC_READABLE;
	}

	if (what & CURL_POLL_OUT) {
		events |= ASYNC_WRITABLE;
	}

	reactor_handle_t * socket_poll = reactor_socket_new_fn((php_socket_t) socket_fd, events);

	if (socket_poll == NULL) {
		return CURLM_BAD_SOCKET;
	}

	async_resume_when(&context->resume, socket_poll, true, multi_poll_callback);

	if (EG(exception)) {
		OBJ_RELEASE(&socket_poll->std);
		zend_exception_to_warning("Failed to add poll callback: %s", true);
		return CURLM_BAD_SOCKET;
	}

	zval z_poll;
	ZVAL_OBJ(&z_poll, &socket_poll->std);
	zend_hash_index_add(context->poll_list, socket_fd, &z_poll);

	curl_multi_assign(curl_multi_handle, socket_fd, socket_poll);

	reactor_add_handle(socket_poll);

	return 0;
}

CURLMcode curl_async_wait(CURLM* multi_handle, int timeout_ms, int* numfds)
{
	curl_async_context * context = (curl_async_context *) async_resume_new_ex(NULL, sizeof(curl_async_context));

	if (context == NULL) {
        return CURLM_OUT_OF_MEMORY;
    }

	int result = CURLM_OK;
	bool is_bailout = false;

#define IF_NULL_RETURN(expr) if (UNEXPECTED((expr) == NULL)) { result = CURLM_OUT_OF_MEMORY; goto finally; }

	zend_try {

		context->curl_multi_handle = multi_handle;

		reactor_handle_t * timer = reactor_timer_new_fn(timeout_ms, false);
		IF_NULL_RETURN(timer);

		async_resume_when(&context->resume, timer, true, async_resume_when_callback_timeout);

		if (UNEXPECTED(EG(exception))) {
            result = CURLM_INTERNAL_ERROR;
			OBJ_RELEASE(&timer->std);
            goto finally;
        }

		curl_multi_setopt(multi_handle, CURLMOPT_SOCKETDATA, context);
		curl_multi_setopt(multi_handle, CURLMOPT_SOCKETFUNCTION, multi_socket_cb);
		curl_multi_setopt(multi_handle, CURLMOPT_TIMERFUNCTION, multi_timer_cb);

		async_await(&context->resume);
	} zend_catch {
		is_bailout = true;
		goto finally;
bailout:
		zend_bailout();
    } zend_end_try();

finally:

	curl_multi_setopt(multi_handle, CURLMOPT_SOCKETFUNCTION, NULL);
	curl_multi_setopt(multi_handle, CURLMOPT_TIMERFUNCTION, NULL);
	curl_multi_setopt(multi_handle, CURLMOPT_SOCKETDATA, NULL);

	if (context->poll_list != NULL) {
		zend_array_destroy(context->poll_list);
	}

	// Calculate the number of file descriptors that are ready
	result = async_resume_get_ready_poll_handles(&context->resume);

	OBJ_RELEASE(&context->resume.std);

	if (is_bailout) {
		goto bailout;
	}

	return result;
}

//=============================================================
#pragma endregion
//=============================================================