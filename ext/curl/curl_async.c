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
#include <async/php_reactor.h>

ZEND_TLS CURLM * curl_multi_handle = NULL;
ZEND_TLS HashTable * curl_multi_context = NULL;
ZEND_TLS uv_loop_t *loop;

int curl_socket_cb(CURL *easy, curl_socket_t socket_fd, int what, void *user_p, void *socket_data)
{
	switch (what)
	{
		case CURL_POLL_IN:
		{
			break;
		}
		case CURL_POLL_OUT:
		{
			break;
		}
		case CURL_POLL_INOUT:
		{
			break;
		}
		case CURL_POLL_REMOVE:
		{
			break;
		}
		default:
		{
			break;
		}
	}


	uv_poll_t *poll_handle = (uv_poll_t *)socket_data;

	if (what == CURL_POLL_REMOVE) {
		if (poll_handle) {
			uv_poll_stop(poll_handle);
			uv_close((uv_handle_t *)poll_handle, free);
		}

		return 0;
	}

	if (!poll_handle) {
		poll_handle = (uv_poll_t *)malloc(sizeof(uv_poll_t));
		uv_poll_init(loop, poll_handle, socket_fd);
		curl_multi_assign(curl_multi_handle, socket_fd, poll_handle);
	}

	int uv_events = 0;

	if (what & CURL_POLL_IN) {
		uv_events |= UV_READABLE;
	}

	if (what & CURL_POLL_OUT) {
		uv_events |= UV_WRITABLE;
	}

	uv_poll_start(poll_handle, uv_events, [](uv_poll_t *handle, int status, int events) {
		int action = 0;

		if (events & UV_READABLE) {
			action |= CURL_CSELECT_IN;
		}

		if (events & UV_WRITABLE) {
			action |= CURL_CSELECT_OUT;
		}

		curl_multi_socket_action(curl_multi_handle, handle->io_watcher.fd, action, NULL);
	});

	return 0;
}

int curl_timer_cb(CURLM *multi, long timeout_ms, void *userp) {

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