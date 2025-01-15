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
#ifndef CURL_H
#define CURL_H

#include <async/php_layer/notifier.h>
#include <async/php_layer/resume.h>
#include <curl/curl.h>

typedef struct _curl_async_context curl_async_context;

struct _curl_async_context {
	CURLM * curl_multi_handle;
	reactor_handle_t * timer;
	zend_object * callback;
	async_resume_t * resume;
	reactor_notifier_t * curl_notifier;
	HashTable * poll_list;
};

void curl_register_notifier(void);
void curl_async_setup(void);
void curl_async_shutdown(void);

CURLcode curl_async_perform(CURL* curl);

CURLMcode curl_async_wait(
	CURLM* multi_handle,
	struct curl_waitfd extra_fds[],
	unsigned int extra_nfds,
	int timeout_ms,
	int* ret);

#endif //CURL_H
