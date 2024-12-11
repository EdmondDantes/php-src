
#ifndef PHP_ASYNC_H
#define PHP_ASYNC_H

#include "php.h"
#include "zend_types.h"
#include "zend_atomic.h"
#include "zend_API.h"
#include "zend_weakrefs.h"
#include "zend_interfaces.h"
#include "php_async_arginfo.h"
#include "internal/async_api.h"
#include "internal/zend_engine.h"
#include "uv.h"


typedef struct _async_completion_publisher_s async_completion_publisher_t;
typedef struct _async_completion_binded_object_s async_completion_binded_object_t;

typedef enum {
	ASYNC_COMPLETION_RESOLV = 1,
	ASYNC_COMPLETION_REJECT = 2,
	ASYNC_COMPLETION_CANCEL = 3,
	ASYNC_COMPLETION_FINALLY = 4
} ASYNC_COMPLETION_ACTION;


struct _async_completion_binded_object_s {
	zval *weak_reference;
	ASYNC_COMPLETION_ACTION action;
};

// Future structure
struct _async_completion_publisher_s {
	zend_object std;
	/**
	 * An array of objects associated with completion events.
	 * It is an array of async_completion_binded_object_t structures.
	 **/
	HashTable* bindedObjects;
};


#endif // PHP_ASYNC_H

