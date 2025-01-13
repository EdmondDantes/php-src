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
#include <async/php_layer/zend_common.h>

ZEND_TLS CURLM * curl_multi_handle = NULL;
ZEND_TLS HashTable * curl_multi_context = NULL;
ZEND_TLS uv_loop_t *loop;

void poll_callback(void * callback, reactor_notifier_t *notifier, const zval* z_event, const zval* error)
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

	CURLMsg *msg;

	while ((msg = curl_multi_info_read(curl_multi_handle, NULL))) {
		if (msg->msg == CURLMSG_DONE) {

			curl_multi_remove_handle(curl_multi_handle, msg->easy_handle);

			const zval * resume = zend_hash_index_find(curl_multi_context, (zend_ulong) msg->easy_handle);

			if (resume == NULL) {
				continue;
			}

			zval result;
			ZVAL_LONG(&result, msg->data.result);
			async_resume_fiber((async_resume_t *) Z_OBJ_P(resume), &result, NULL);
		}
	}
}

int curl_socket_cb(CURL *curl, const curl_socket_t socket_fd, const int what, void *user_p, void *socket_poll)
{
	const zval * resume = zend_hash_index_find(curl_multi_context, (zend_ulong) curl);

	if (resume == NULL) {
		return 0;
	}

	if (what == CURL_POLL_REMOVE) {

		if (socket_poll == NULL) {
			return 0;
		}

		curl_multi_assign(curl_multi_handle, ((reactor_poll_t *) socket_poll)->socket, NULL);
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

		zval callback;
		ZVAL_PTR(&callback, poll_callback);
		async_notifier_add_callback(socket_poll, &callback);

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

int curl_timer_cb(CURLM *multi, long timeout_ms, void *user_p) {

	static uv_timer_t timer;

	if (timeout_ms < 0) {
		uv_timer_stop(&timer);
		return 0;
	}

	if (!uv_is_active((uv_handle_t *)&timer)) {
		uv_timer_init(loop, &timer);
	}

	uv_timer_start(&timer, [](uv_timer_t *handle) {
		curl_multi_socket_action(curl_multi_handle, CURL_SOCKET_TIMEOUT, 0, NULL);
	}, timeout_ms, 0);

	return 0;
}

void async_curl_setup(void)
{
	curl_multi_handle = curl_multi_init();
	curl_multi_setopt(curl_multi_handle, CURLMOPT_SOCKETFUNCTION, curl_socket_cb);
	curl_multi_setopt(curl_multi_handle, CURLMOPT_TIMERFUNCTION, curl_timer_cb);
	curl_multi_context = zend_new_array(8);
}

void async_curl_shutdown(void)
{
	if (curl_multi_handle != NULL) {
		curl_multi_cleanup(curl_multi_handle);
		curl_multi_handle = NULL;
	}

	if (curl_multi_context != NULL) {
		zend_array_destroy(curl_multi_context);
		curl_multi_context = NULL;
	}
}

CURLcode curl_async_perform(CURL* curl)
{
	// Add curl handle to curl_multi_context
	zval z_resume;
	async_resume_t * resume = async_resume_new(NULL);

	if (resume == NULL) {
		return CURLE_FAILED_INIT;
	}

	ZVAL_OBJ(&z_resume, &resume->std);

	// add z_resume to curl_multi_context
	if (zend_hash_index_update(curl_multi_context, (zend_ulong) curl, &z_resume) == NULL) {
		zval_ptr_dtor(&z_resume);
		return CURLE_FAILED_INIT;
	}

	async_await(resume);

	ZEND_ASSERT(GC_REFCOUNT(&resume->std) == 1 && "Memory leak detected. The resume object should have only one reference");
	zend_hash_index_del(curl_multi_context, (zend_ulong) curl);

	return CURLE_OK;
}