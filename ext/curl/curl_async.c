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
#include <uv.h>
#include <zend_exceptions.h>
#include <async/php_reactor.h>
#include <async/php_layer/callback.h>
#include <async/php_layer/zend_common.h>

static zend_class_entry * curl_ce_notifier = NULL;
ZEND_TLS CURLM * curl_multi_handle = NULL;
ZEND_TLS HashTable * curl_multi_resume_list = NULL;
ZEND_TLS reactor_handle_t * timer = NULL;
ZEND_TLS zend_object * poll_callback_obj = NULL;
ZEND_TLS zend_object * timer_callback_obj = NULL;

static void process_curl_completed_handles(void)
{
	CURLMsg *msg;

	while ((msg = curl_multi_info_read(curl_multi_handle, NULL))) {
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

static void poll_callback(zend_object * callback, reactor_notifier_t *notifier, const zval* z_event, const zval* error)
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

static int curl_socket_cb(CURL *curl, const curl_socket_t socket_fd, const int what, void *user_p, void *socket_poll)
{
	const zval * resume = zend_hash_index_find(curl_multi_resume_list, (zend_ulong) curl);
	const curl_async_context * context = (curl_async_context *) user_p;
	CURLM* multi_handle = context != NULL ? context->curl_multi_handle : NULL;

	if (resume == NULL) {
		return 0;
	}

	if (what == CURL_POLL_REMOVE) {

		if (socket_poll == NULL) {
			return 0;
		}

		curl_multi_assign(multi_handle, ((reactor_poll_t *) socket_poll)->socket, NULL);
		reactor_remove_handle_fn((reactor_handle_t *) socket_poll);
		OBJ_RELEASE(&((reactor_handle_t *) socket_poll)->std);
		return 0;
	}

	if (socket_poll == NULL) {

		zend_long events = 0;

		if (what & CURL_POLL_IN) {
			events |= UV_READABLE;
		}

		if (what & CURL_POLL_OUT) {
			events |= UV_WRITABLE;
		}

		socket_poll = reactor_socket_new_fn((php_socket_t) socket_fd, events);

		if (socket_poll == NULL) {
			return CURLM_BAD_SOCKET;
		}

		if (poll_callback_obj == NULL) {
            poll_callback_obj = async_callback_new(poll_callback);

            if (poll_callback_obj == NULL) {
                OBJ_RELEASE(&((reactor_handle_t *) socket_poll)->std);
                return CURLM_BAD_SOCKET;
            }
        }

		zval callback;
		ZVAL_OBJ(&callback, poll_callback_obj);
		async_notifier_add_callback(socket_poll, &callback);

		if (EG(exception)) {
			OBJ_RELEASE(&((reactor_handle_t *) socket_poll)->std);
			zend_exception_to_warning("Failed to add poll callback: %s", true);
			return CURLM_BAD_SOCKET;
		}

		curl_multi_assign(multi_handle, socket_fd, socket_poll);

		reactor_add_handle(socket_poll);
	}

	return 0;
}

static void timer_callback(zend_object * callback, reactor_notifier_t *notifier, const zval* z_event, const zval* error)
{
	curl_multi_socket_action(curl_multi_handle, CURL_SOCKET_TIMEOUT, 0, NULL);
	process_curl_completed_handles();
}

static void curl_event_cb(zend_object * callback, reactor_notifier_t *notifier, const zval* z_event, const zval* error)
{
	if (notifier->std.ce == async_ce_timer_handle) {
        timer_callback(callback, notifier, z_event, error);
    } else {
    	poll_callback(callback, notifier, z_event, error);
    }
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

void curl_register_notifier(void)
{
	zend_class_entry ce;

	if (curl_ce_notifier != NULL) {
        return;
    }

	INIT_NS_CLASS_ENTRY(ce, "Async", "CurlNotifier", NULL);

	curl_ce_notifier = zend_register_internal_class_with_flags(
		&ce, async_ce_notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE
    );
}

static bool curl_notifier_remove_callback(reactor_notifier_t * notifier, zval * callback)
{
	if (Z_TYPE_P(callback) == IS_OBJECT && Z_OBJ_P(callback)->ce == async_ce_resume) {

    }

	return true;
}

reactor_notifier_t * curl_notifier_new(void)
{
	DEFINE_ZEND_INTERNAL_OBJECT(reactor_notifier_t, notifier, curl_ce_notifier);

    if (notifier == NULL) {
        return NULL;
    }

	notifier->remove_callback_fn = curl_notifier_remove_callback;

    return notifier;
}

void curl_async_setup(void)
{
	if (curl_multi_handle != NULL) {
		return;
	}

	curl_register_notifier();

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

	return CURLE_OK;
}

CURLMcode curl_async_wait(
	CURLM* multi_handle,
	struct curl_waitfd extra_fds[],
	unsigned int extra_nfds,
	int timeout_ms,
	int* ret)
{
	curl_async_context * context = emalloc(sizeof(curl_async_context));

	if (context == NULL) {
        return CURLM_OUT_OF_MEMORY;
    }

	int result = CURLM_OK;
	bool is_bailout = false;

#define IF_NULL_RETURN(expr) if (UNEXPECTED((expr) == NULL)) { result = CURLM_OUT_OF_MEMORY; goto finally; }

	zend_try {
		context->curl_multi_handle = multi_handle;
		context->timer = reactor_timer_new_fn(timeout_ms, false);
		IF_NULL_RETURN(context->timer);

		context->callback = async_callback_new(curl_event_cb);
		IF_NULL_RETURN(context->callback);

		context->curl_notifier = curl_notifier_new();
		IF_NULL_RETURN(context->curl_notifier);

		context->resume = async_resume_new(NULL);
		IF_NULL_RETURN(context->resume);

		zval z_callback;
		ZVAL_OBJ(&z_callback, context->callback);
		async_notifier_add_callback(&context->timer->std, &z_callback);
		async_resume_when(context->resume, context->curl_notifier, true, async_resume_when_callback_resolve);
		async_resume_when(context->resume, context->timer, true, async_resume_when_callback_timeout);

		if (UNEXPECTED(EG(exception))) {
            result = CURLM_INTERNAL_ERROR;
            goto finally;
        }

		curl_multi_setopt(multi_handle, CURLMOPT_SOCKETDATA, context);
		curl_multi_setopt(multi_handle, CURLMOPT_SOCKETFUNCTION, curl_socket_cb);
		curl_multi_setopt(multi_handle, CURLMOPT_TIMERFUNCTION, curl_timer_cb);

		async_await(context->resume);
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

	if (context->resume != NULL) {
		OBJ_RELEASE(&context->resume->std);
	}

	if (context->timer != NULL) {
        OBJ_RELEASE(&context->timer->std);
    }

	if (context->callback != NULL) {
		OBJ_RELEASE(context->callback);
	}

	efree(context);

	if (is_bailout) {
		goto bailout;
	}

	return result;
}