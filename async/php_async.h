
//
// Condition if fiber
//
#ifndef PHP_ASYNC_H
#define PHP_ASYNC_H

// #define ASYNC_IS_FIBER !EG(active_fiber)
#define ASYNC_IS_FIBER true

//
// Intercepts a function call and replaces it with a call to another function if a fiber is active.
//
#define ASYNC_HOOK_IF_FIBER_AND_RETURN(FUN_CALL) \
if (!EG(active_fiber)) {                         \
    return FUN_CALL;                             \
}

#endif //PHP_ASYNC_H
