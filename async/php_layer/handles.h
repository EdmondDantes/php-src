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
#ifndef HANDLES_H
#define HANDLES_H

typedef enum {
	ASYNC_UNKNOWN = 0,
	ASYNC_FILE = 1,
	ASYNC_SOCKET = 2,
	ASYNC_TIMER = 3,
	ASYNC_SIGNAL = 4,
	ASYNC_PIPE = 5,
	ASYNC_TTY = 6,
	ASYNC_FILE_SYSTEM = 7,
	ASYNC_PROCESS = 8,
	ASYNC_IDLE = 9,
	ASYNC_GETADDRINFO = 10,
	ASYNC_GETNAMEINFO = 11,

	ASYNC_CUSTOM_TYPE = 128
} ASYNC_HANDLE_TYPE;


#endif //HANDLES_H
