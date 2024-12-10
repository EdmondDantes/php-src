//
// An abstraction over LIBUV that provides functionality for PHPCore.
//

#ifndef ENGINE_H
#define ENGINE_H

#ifdef PHP_WIN32
#include <Winsock2.h>
#include <Mswsock.h>
#include <psapi.h>
#include <Iphlpapi.h>
#endif

#ifndef PHP_UV_DTRACE
#define PHP_UV_DTRACE 0
#endif

#if PHP_UV_DTRACE >= 1
#include <dtrace.h>
#include <sys/sdt.h>
#include "phpuv_dtrace.h"
#define PHP_UV_PROBE(PROBE) PHPUV_TRACE_##PROBE();
#else
#define PHP_UV_PROBE(PROBE)
#endif

#include "php.h"
#include "php_network.h"
#include "php_streams.h"

#include <Zend/zend.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_globals.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_list.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>

#ifdef HAVE_SOCKETS
#include "ext/sockets/php_sockets.h"
#endif

// Curl
#ifdef HAVE_CURL
#include "ext/curl/php_curl.h"
#include <curl/curl.h>
#include <curl/easy.h>
#endif

#if PHP_VERSION_ID >= 80000
#define TSRMLS_C
#define TSRMLS_CC
#define TSRMLS_D
#define TSRMLS_DC
#define TSRMLS_FETCH_FROM_CTX(ctx)
#define TSRMLS_SET_CTX(ctx)
#endif

#define PHP_ASYNC_FD_NONE 	-1
#define PHP_ASYNC_FD_FAILED -1
#define PHP_ASYNC_MAIN_FIBER 0

typedef struct {
    zend_fiber *fiber;
    int fd;
} io_wait_context_t;

typedef struct {
    io_wait_context_t context;
    uv_poll_t poll;
} io_poll_t;

#endif //ENGINE_H
