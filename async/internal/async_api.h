
//
// Condition if fiber
//
#ifndef ASYNC_API_H
#define ASYNC_API_H

#include "php.h"
#include "zend_types.h"
#include "zend_atomic.h"
#include "uv.h"

//
// Structures block start
//

typedef struct _async_future_s async_future;
typedef struct _async_deferred_resume_s async_deferred_resume_t;
typedef struct _async_await_context_s async_await_context_t;
typedef struct _async_event_handler_s async_event_handler_t;

// Future statuses
typedef enum {
    ASYNC_FUTURE_PENDING     = 0,
    ASYNC_FUTURE_RESOLVED    = 1,
    ASYNC_FUTURE_REJECTED    = 2,
    ASYNC_FUTURE_CANCELLED   = 3
} ASYNC_FUTURE_STATUS;

// Future structure
struct _async_future_s {
    zend_atomic_int status;
    zval *result;
};

/**
 * DeferredResume structure
 */
struct _async_deferred_resume_s {
    /* PHP object handle. */
    zend_object std;
    /* Deferred state. */
    zend_atomic_int status;
    /* Fiber that will be resumed. */
    zend_fiber *fiber;
};

//
// EventHandler structure
// This structure is used to handle events in the event loop.
//

typedef enum {
    ASYNC_CONTEXT_READY    = 0,
    ASYNC_CONTEXT_USED     = 1,
    ASYNC_CONTEXT_DISPOSED = 2,
} ASYNC_CONTEXT_STATUS;

typedef void (*async_event_handler_method)(async_event_handler_t *event_handler);

struct _async_event_handler_s {
    async_event_handler_method callback;
    async_event_handler_method dispose;
    zval *event_descriptor;
    async_await_context_t *context;
    uv_handle_t uv_handle;
};

//
// AwaitContext structure
// This structure is used to handle the context of the await.
//

typedef void (*async_context_method)(async_await_context_t *await_context);

struct _async_await_context_s {
    zend_atomic_int status;
    async_deferred_resume_t *deferred_resume;
    zval *context;
    async_context_method apply;
    async_context_method reset;
    async_context_method dispose;
    size_t event_handler_count;
    async_event_handler_t event_handlers[];
};

//
// Structures block end
//

// #define ASYNC_IS_FIBER !EG(active_fiber)
#define ASYNC_IS_FIBER true

//
// Intercepts a function call and replaces it with a call to another function if a fiber is active.
//
#define ASYNC_HOOK_IF_FIBER_AND_RETURN(FUN_CALL) \
if (!EG(active_fiber)) {                         \
    return FUN_CALL;                             \
}

#endif //ASYNC_API_H
