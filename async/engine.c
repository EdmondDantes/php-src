//
// An abstraction over LIBUV that provides functionality for PHPCore.
//

#include "engine.h"
#include "zend_engine.h"
#include <threads.h>

thread_local uv_loop_t *thread_loop = NULL;

uv_loop_t *get_loop() {

	if (!thread_loop) {

		thread_loop = malloc(sizeof(uv_loop_t));

		if (uv_loop_init(thread_loop) != 0) {
			fprintf(stderr, "Failed to initialize event loop\n");
			free(thread_loop);
			thread_loop = INVALID_POINTER;
			return INVALID_POINTER;
		}
	}

	return thread_loop;
}

void run_event_loop_once() {

	uv_loop_t *loop = get_loop();

	if (!loop || loop == INVALID_POINTER) {
		fprintf(stderr, "Failed to retrieve thread-local loop\n");
		return;
	}

	if (!uv_loop_alive(loop)) {
		uv_run(loop, UV_RUN_ONCE);
	}
}

void free_loop() {
	if (thread_loop) {
		uv_loop_close(thread_loop);
		free(thread_loop);
		thread_loop = NULL;
	}
}

/*
 * The method converts a ZVal structure into a real system input/output descriptor.
 * If you need to support various PHP libraries, you can add additional code to this function.
 * */
/* {{{ php_async_zval_to_fd */
PHPAPI php_socket_t php_async_zval_to_fd(zval *zval_ptr)
{
	if (Z_TYPE_P(zval_ptr) == IS_RESOURCE) {
          // Support Resources
		php_stream *stream = zend_fetch_resource_ex(zval_ptr, NULL, php_file_le_stream());

		if (!stream) {
    		return PHP_ASYNC_FD_FAILED;
		}

        php_socket_t fd = PHP_ASYNC_FD_NONE;

		if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void*)&fd, 1) == SUCCESS
            && fd >= 0) {
    		return fd;
		}

		return PHP_ASYNC_FD_FAILED;

	} else if (Z_TYPE_P(zval_ptr) == IS_OBJECT) {

          //
          // socket_class_entry can be NULL if the class is not found.
          // So it means that the class is not loaded.
          //

#ifdef HAVE_SOCKETS

		  GET_CLASS_ENTRY("Socket", socket_class_entry);

          if(socket_class_entry != NULL && Z_OBJCE_P(zval_ptr) == socket_class_entry) {
              return Z_SOCKET_P(zval_ptr)->bsd_socket;
          }

#endif

#ifdef HAVE_CURL

          GET_CLASS_ENTRY("CurlHandle", curl_class_entry);

          if(curl_class_entry != NULL && Z_OBJCE_P(zval_ptr) == curl_class_entry) {

          		php_curl *curl_obj = (php_curl *)Z_OBJ_P(zval_ptr);

          	if (curl_obj != NULL && curl_obj->cp != NULL) {
          		curl_socket_t sock_fd;

          		if (curl_easy_getinfo(curl_obj->cp, CURLINFO_ACTIVESOCKET, &sock_fd) == CURLE_OK) {
          			return sock_fd;
          		}
          	}
          }

#endif
	}

    return PHP_ASYNC_FD_FAILED;
}
/* }}} */