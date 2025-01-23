/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Zend Technologies Ltd. (http://www.zend.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Aaron Piotrowski <aaron@trowski.com>                        |
   |          Martin Schröder <m.schroeder2007@gmail.com>                 |
   +----------------------------------------------------------------------+
*/

#include "zend.h"
#include "zend_API.h"
#include "zend_gc.h"
#include "zend_ini.h"
#include "zend_variables.h"
#include "zend_vm.h"
#include "zend_exceptions.h"
#include "zend_builtin_functions.h"
#include "zend_observer.h"
#include "zend_mmap.h"
#include "zend_compile.h"
#include "zend_closures.h"
#include "zend_generators.h"

#include "zend_fibers.h"

#ifdef PHP_ASYNC
#include "async/php_async.h"
#endif

#include "php.h"

#include "zend_fibers_arginfo.h"

#ifdef HAVE_VALGRIND
# include <valgrind/valgrind.h>
#endif

#ifdef ZEND_FIBER_UCONTEXT
# include <ucontext.h>
#endif

#ifndef ZEND_WIN32
# include <unistd.h>
# include <sys/mman.h>
# include <limits.h>

# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#  define MAP_ANONYMOUS MAP_ANON
# endif

/* FreeBSD require a first (i.e. addr) argument of mmap(2) is not NULL
 * if MAP_STACK is passed.
 * http://www.FreeBSD.org/cgi/query-pr.cgi?pr=158755 */
# if !defined(MAP_STACK) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#  undef MAP_STACK
#  define MAP_STACK 0
# endif

# ifndef MAP_FAILED
#  define MAP_FAILED ((void * ) -1)
# endif
#endif

#ifdef __SANITIZE_ADDRESS__
# include <sanitizer/asan_interface.h>
# include <sanitizer/common_interface_defs.h>
#endif

# if defined __CET__
#  include <cet.h>
#  define SHSTK_ENABLED (__CET__ & 0x2)
#  define BOOST_CONTEXT_SHADOW_STACK (SHSTK_ENABLED && SHADOW_STACK_SYSCALL)
#  define __NR_map_shadow_stack 451
# ifndef SHADOW_STACK_SET_TOKEN
#  define SHADOW_STACK_SET_TOKEN 0x1
#endif
#endif

/* Encapsulates the fiber C stack with extension for debugging tools. */
struct _zend_fiber_stack {
	void *pointer;
	size_t size;

#ifdef HAVE_VALGRIND
	unsigned int valgrind_stack_id;
#endif

#ifdef __SANITIZE_ADDRESS__
	const void *asan_pointer;
	size_t asan_size;
#endif

#ifdef ZEND_FIBER_UCONTEXT
	/* Embedded ucontext to avoid unnecessary memory allocations. */
	ucontext_t ucontext;
#elif BOOST_CONTEXT_SHADOW_STACK
	/* Shadow stack: base, size */
	void *ss_base;
	size_t ss_size;
#endif
};

/* Zend VM state that needs to be captured / restored during fiber context switch. */
typedef struct _zend_fiber_vm_state {
	zend_vm_stack vm_stack;
	zval *vm_stack_top;
	zval *vm_stack_end;
	size_t vm_stack_page_size;
	zend_execute_data *current_execute_data;
	int error_reporting;
	uint32_t jit_trace_num;
	JMP_BUF *bailout;
	zend_fiber *active_fiber;
#ifdef ZEND_CHECK_STACK_LIMIT
	void *stack_base;
	void *stack_limit;
#endif
} zend_fiber_vm_state;

static zend_always_inline void zend_fiber_capture_vm_state(zend_fiber_vm_state *state)
{
	state->vm_stack = EG(vm_stack);
	state->vm_stack_top = EG(vm_stack_top);
	state->vm_stack_end = EG(vm_stack_end);
	state->vm_stack_page_size = EG(vm_stack_page_size);
	state->current_execute_data = EG(current_execute_data);
	state->error_reporting = EG(error_reporting);
	state->jit_trace_num = EG(jit_trace_num);
	state->bailout = EG(bailout);
	state->active_fiber = EG(active_fiber);
#ifdef ZEND_CHECK_STACK_LIMIT
	state->stack_base = EG(stack_base);
	state->stack_limit = EG(stack_limit);
#endif
}

static zend_always_inline void zend_fiber_restore_vm_state(zend_fiber_vm_state *state)
{
	EG(vm_stack) = state->vm_stack;
	EG(vm_stack_top) = state->vm_stack_top;
	EG(vm_stack_end) = state->vm_stack_end;
	EG(vm_stack_page_size) = state->vm_stack_page_size;
	EG(current_execute_data) = state->current_execute_data;
	EG(error_reporting) = state->error_reporting;
	EG(jit_trace_num) = state->jit_trace_num;
	EG(bailout) = state->bailout;
	EG(active_fiber) = state->active_fiber;
#ifdef ZEND_CHECK_STACK_LIMIT
	EG(stack_base) = state->stack_base;
	EG(stack_limit) = state->stack_limit;
#endif
}

#ifdef ZEND_FIBER_UCONTEXT
ZEND_TLS zend_fiber_transfer *transfer_data;
#else
/* boost_context_data is our customized definition of struct transfer_t as
 * provided by boost.context in fcontext.hpp:
 *
 * typedef void* fcontext_t;
 *
 * struct transfer_t {
 *     fcontext_t fctx;
 *     void *data;
 * }; */

typedef struct {
	void *handle;
	zend_fiber_transfer *transfer;
} boost_context_data;

/* These functions are defined in assembler files provided by boost.context (located in "Zend/asm"). */
extern void *make_fcontext(void *sp, size_t size, void (*fn)(boost_context_data));
extern ZEND_INDIRECT_RETURN boost_context_data jump_fcontext(void *to, zend_fiber_transfer *transfer);
#endif

ZEND_API zend_class_entry *zend_ce_fiber;
static zend_class_entry *zend_ce_fiber_error;
#ifdef PHP_ASYNC
ZEND_API zend_class_entry *zend_ce_fiber_context;
#endif

static zend_object_handlers zend_fiber_handlers;

static zend_function zend_fiber_function = { ZEND_INTERNAL_FUNCTION };

ZEND_TLS uint32_t zend_fiber_switch_blocking = 0;

#define ZEND_FIBER_DEFAULT_PAGE_SIZE 4096

static size_t zend_fiber_get_page_size(void)
{
	static size_t page_size = 0;

	if (!page_size) {
		page_size = zend_get_page_size();
		if (!page_size || (page_size & (page_size - 1))) {
			/* anyway, we have to return a valid result */
			page_size = ZEND_FIBER_DEFAULT_PAGE_SIZE;
		}
	}

	return page_size;
}

static zend_fiber_stack *zend_fiber_stack_allocate(size_t size)
{
	void *pointer;
	const size_t page_size = zend_fiber_get_page_size();
	const size_t minimum_stack_size = page_size + ZEND_FIBER_GUARD_PAGES * page_size;

	if (size < minimum_stack_size) {
		zend_throw_exception_ex(NULL, 0, "Fiber stack size is too small, it needs to be at least %zu bytes", minimum_stack_size);
		return NULL;
	}

	const size_t stack_size = (size + page_size - 1) / page_size * page_size;
	const size_t alloc_size = stack_size + ZEND_FIBER_GUARD_PAGES * page_size;

#ifdef ZEND_WIN32
	pointer = VirtualAlloc(0, alloc_size, MEM_COMMIT, PAGE_READWRITE);

	if (!pointer) {
		DWORD err = GetLastError();
		char *errmsg = php_win32_error_to_msg(err);
		zend_throw_exception_ex(NULL, 0, "Fiber stack allocate failed: VirtualAlloc failed: [0x%08lx] %s", err, errmsg[0] ? errmsg : "Unknown");
		php_win32_error_msg_free(errmsg);
		return NULL;
	}

# if ZEND_FIBER_GUARD_PAGES
	DWORD protect;

	if (!VirtualProtect(pointer, ZEND_FIBER_GUARD_PAGES * page_size, PAGE_READWRITE | PAGE_GUARD, &protect)) {
		DWORD err = GetLastError();
		char *errmsg = php_win32_error_to_msg(err);
		zend_throw_exception_ex(NULL, 0, "Fiber stack protect failed: VirtualProtect failed: [0x%08lx] %s", err, errmsg[0] ? errmsg : "Unknown");
		php_win32_error_msg_free(errmsg);
		VirtualFree(pointer, 0, MEM_RELEASE);
		return NULL;
	}
# endif
#else
	pointer = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

	if (pointer == MAP_FAILED) {
		zend_throw_exception_ex(NULL, 0, "Fiber stack allocate failed: mmap failed: %s (%d)", strerror(errno), errno);
		return NULL;
	}

#if defined(MADV_NOHUGEPAGE)
	/* Multiple reasons to fail, ignore all errors only needed
	 * for linux < 6.8 */
	(void) madvise(pointer, alloc_size, MADV_NOHUGEPAGE);
#endif

	zend_mmap_set_name(pointer, alloc_size, "zend_fiber_stack");

# if ZEND_FIBER_GUARD_PAGES
	if (mprotect(pointer, ZEND_FIBER_GUARD_PAGES * page_size, PROT_NONE) < 0) {
		zend_throw_exception_ex(NULL, 0, "Fiber stack protect failed: mprotect failed: %s (%d)", strerror(errno), errno);
		munmap(pointer, alloc_size);
		return NULL;
	}
# endif
#endif

	zend_fiber_stack *stack = emalloc(sizeof(zend_fiber_stack));

	stack->pointer = (void *) ((uintptr_t) pointer + ZEND_FIBER_GUARD_PAGES * page_size);
	stack->size = stack_size;

#if !defined(ZEND_FIBER_UCONTEXT) && BOOST_CONTEXT_SHADOW_STACK
	/* shadow stack saves ret address only, need less space */
	stack->ss_size= stack_size >> 5;

	/* align shadow stack to 8 bytes. */
	stack->ss_size = (stack->ss_size + 7) & ~7;

	/* issue syscall to create shadow stack for the new fcontext */
	/* SHADOW_STACK_SET_TOKEN option will put "restore token" on the new shadow stack */
	stack->ss_base = (void *)syscall(__NR_map_shadow_stack, 0, stack->ss_size, SHADOW_STACK_SET_TOKEN);

	if (stack->ss_base == MAP_FAILED) {
		zend_throw_exception_ex(NULL, 0, "Fiber shadow stack allocate failed: mmap failed: %s (%d)", strerror(errno), errno);
		return NULL;
	}
#endif

#ifdef VALGRIND_STACK_REGISTER
	uintptr_t base = (uintptr_t) stack->pointer;
	stack->valgrind_stack_id = VALGRIND_STACK_REGISTER(base, base + stack->size);
#endif

#ifdef __SANITIZE_ADDRESS__
	stack->asan_pointer = stack->pointer;
	stack->asan_size = stack->size;
#endif

	return stack;
}

static void zend_fiber_stack_free(zend_fiber_stack *stack)
{
#ifdef VALGRIND_STACK_DEREGISTER
	VALGRIND_STACK_DEREGISTER(stack->valgrind_stack_id);
#endif

	const size_t page_size = zend_fiber_get_page_size();

	void *pointer = (void *) ((uintptr_t) stack->pointer - ZEND_FIBER_GUARD_PAGES * page_size);

#ifdef __SANITIZE_ADDRESS__
	/* If another mmap happens after unmapping, it may trigger the stale stack red zones
	 * so we have to unpoison it before unmapping. */
	ASAN_UNPOISON_MEMORY_REGION(pointer, stack->size + ZEND_FIBER_GUARD_PAGES * page_size);
#endif

#ifdef ZEND_WIN32
	VirtualFree(pointer, 0, MEM_RELEASE);
#else
	munmap(pointer, stack->size + ZEND_FIBER_GUARD_PAGES * page_size);
#endif

#if !defined(ZEND_FIBER_UCONTEXT) && BOOST_CONTEXT_SHADOW_STACK
	munmap(stack->ss_base, stack->ss_size);
#endif

	efree(stack);
}

#ifdef ZEND_CHECK_STACK_LIMIT
ZEND_API void* zend_fiber_stack_limit(zend_fiber_stack *stack)
{
	zend_ulong reserve = EG(reserved_stack_size);

#ifdef __APPLE__
	/* On Apple Clang, the stack probing function ___chkstk_darwin incorrectly
	 * probes a location that is twice the entered function's stack usage away
	 * from the stack pointer, when using an alternative stack.
	 * https://openradar.appspot.com/radar?id=5497722702397440
	 */
	reserve = reserve * 2;
#endif

	/* stack->pointer is the end of the stack */
	return (int8_t*)stack->pointer + reserve;
}

ZEND_API void* zend_fiber_stack_base(zend_fiber_stack *stack)
{
	return (void*)((uintptr_t)stack->pointer + stack->size);
}
#endif

#ifdef ZEND_FIBER_UCONTEXT
static ZEND_NORETURN void zend_fiber_trampoline(void)
#else
static ZEND_NORETURN void zend_fiber_trampoline(boost_context_data data)
#endif
{
	/* Initialize transfer struct with a copy of passed data. */
#ifdef ZEND_FIBER_UCONTEXT
	zend_fiber_transfer transfer = *transfer_data;
#else
	zend_fiber_transfer transfer = *data.transfer;
#endif

	zend_fiber_context *from = transfer.context;

#ifdef __SANITIZE_ADDRESS__
	__sanitizer_finish_switch_fiber(NULL, &from->stack->asan_pointer, &from->stack->asan_size);
#endif

#ifndef ZEND_FIBER_UCONTEXT
	/* Get the context that resumed us and update its handle to allow for symmetric coroutines. */
	from->handle = data.handle;
#endif

	/* Ensure that previous fiber will be cleaned up (needed by symmetric coroutines). */
	if (from->status == ZEND_FIBER_STATUS_DEAD) {
		zend_fiber_destroy_context(from);
	}

	zend_fiber_context *context = EG(current_fiber_context);

	context->function(&transfer);
	context->status = ZEND_FIBER_STATUS_DEAD;

	/* Final context switch, the fiber must not be resumed afterwards! */
	zend_fiber_switch_context(&transfer);

	/* Abort here because we are in an inconsistent program state. */
	abort();
}

//=========================================================
#pragma region PHP ASYNC API
//=========================================================
#ifdef PHP_ASYNC
static void zend_fiber_invoke_shutdown_handlers(zend_fiber *fiber)
{
    if (fiber->shutdown_handlers == NULL) {
        return;
    }

	zend_exception_save();

    zval *callback;
	zval retval;
	ZVAL_UNDEF(&retval);

    ZEND_HASH_FOREACH_VAL(fiber->shutdown_handlers, callback) {

    	if (Z_TYPE_P(callback) == IS_PTR) {
    		zend_fiber_defer_entry *entry = Z_PTR_P(callback);
    		entry->func(fiber, entry);
    		continue;
    	}

        const char *debug_type = zend_zval_value_name(callback);

    	if (call_user_function(EG(function_table), NULL, callback, &retval, 0, NULL) == FAILURE) {
			php_error_docref(
				NULL,
				E_WARNING,
				"Failed to execute fiber shutdown handler with type: %s",
				debug_type
			);
		}

    	zval_ptr_dtor(&retval);

    	if (EG(exception) != NULL) {
    		zend_exception_save();
    	}

    } ZEND_HASH_FOREACH_END();

    zend_hash_destroy(fiber->shutdown_handlers);
    efree(fiber->shutdown_handlers);
    fiber->shutdown_handlers = NULL;

	zend_exception_restore();
}

#endif
//=========================================================
#pragma endregion
//=========================================================

ZEND_API void zend_fiber_switch_block(void)
{
	++zend_fiber_switch_blocking;
}

ZEND_API void zend_fiber_switch_unblock(void)
{
	ZEND_ASSERT(zend_fiber_switch_blocking && "Fiber switching was not blocked");
	--zend_fiber_switch_blocking;
}

ZEND_API bool zend_fiber_switch_blocked(void)
{
	return zend_fiber_switch_blocking;
}

ZEND_API zend_result zend_fiber_init_context(zend_fiber_context *context, void *kind, zend_fiber_coroutine coroutine, size_t stack_size)
{
	context->stack = zend_fiber_stack_allocate(stack_size);

	if (UNEXPECTED(!context->stack)) {
		return FAILURE;
	}

#ifdef ZEND_FIBER_UCONTEXT
	ucontext_t *handle = &context->stack->ucontext;

	getcontext(handle);

	handle->uc_stack.ss_size = context->stack->size;
	handle->uc_stack.ss_sp = context->stack->pointer;
	handle->uc_stack.ss_flags = 0;
	handle->uc_link = NULL;

	makecontext(handle, (void (*)(void)) zend_fiber_trampoline, 0);

	context->handle = handle;
#else
	// Stack grows down, calculate the top of the stack. make_fcontext then shifts pointer to lower 16-byte boundary.
	void *stack = (void *) ((uintptr_t) context->stack->pointer + context->stack->size);

#if BOOST_CONTEXT_SHADOW_STACK
	// pass the shadow stack pointer to make_fcontext
	// i.e., link the new shadow stack with the new fcontext
	// TODO should be a better way?
	*((unsigned long*) (stack - 8)) = (unsigned long)context->stack->ss_base + context->stack->ss_size;
#endif

	context->handle = make_fcontext(stack, context->stack->size, zend_fiber_trampoline);
	ZEND_ASSERT(context->handle != NULL && "make_fcontext() never returns NULL");
#endif

	context->kind = kind;
	context->function = coroutine;

	// Set status in case memory has not been zeroed.
	context->status = ZEND_FIBER_STATUS_INIT;

	zend_observer_fiber_init_notify(context);

	return SUCCESS;
}

ZEND_API void zend_fiber_destroy_context(zend_fiber_context *context)
{
	zend_observer_fiber_destroy_notify(context);

	if (context->cleanup) {
		context->cleanup(context);
	}

	zend_fiber_stack_free(context->stack);
}

ZEND_API void zend_fiber_switch_context(zend_fiber_transfer *transfer)
{
	zend_fiber_context *from = EG(current_fiber_context);
	zend_fiber_context *to = transfer->context;
	zend_fiber_vm_state state;

	ZEND_ASSERT(to && to->handle && to->status != ZEND_FIBER_STATUS_DEAD && "Invalid fiber context");
	ZEND_ASSERT(from && "From fiber context must be present");
	ZEND_ASSERT(to != from && "Cannot switch into the running fiber context");

	/* Assert that all error transfers hold a Throwable value. */
	ZEND_ASSERT((
		!(transfer->flags & ZEND_FIBER_TRANSFER_FLAG_ERROR) ||
		(Z_TYPE(transfer->value) == IS_OBJECT && (
			zend_is_unwind_exit(Z_OBJ(transfer->value)) ||
			zend_is_graceful_exit(Z_OBJ(transfer->value)) ||
			instanceof_function(Z_OBJCE(transfer->value), zend_ce_throwable)
		))
	) && "Error transfer requires a throwable value");

	zend_observer_fiber_switch_notify(from, to);

	zend_fiber_capture_vm_state(&state);

	to->status = ZEND_FIBER_STATUS_RUNNING;

	if (EXPECTED(from->status == ZEND_FIBER_STATUS_RUNNING)) {
		from->status = ZEND_FIBER_STATUS_SUSPENDED;
	}

	/* Update transfer context with the current fiber before switching. */
	transfer->context = from;

	EG(current_fiber_context) = to;

#ifdef __SANITIZE_ADDRESS__
	void *fake_stack = NULL;
	__sanitizer_start_switch_fiber(
		from->status != ZEND_FIBER_STATUS_DEAD ? &fake_stack : NULL,
		to->stack->asan_pointer,
		to->stack->asan_size);
#endif

#ifdef ZEND_FIBER_UCONTEXT
	transfer_data = transfer;

	swapcontext(from->handle, to->handle);

	/* Copy transfer struct because it might live on the other fiber's stack that will eventually be destroyed. */
	*transfer = *transfer_data;
#else
	boost_context_data data = jump_fcontext(to->handle, transfer);

	/* Copy transfer struct because it might live on the other fiber's stack that will eventually be destroyed. */
	*transfer = *data.transfer;
#endif

	to = transfer->context;

#ifndef ZEND_FIBER_UCONTEXT
	/* Get the context that resumed us and update its handle to allow for symmetric coroutines. */
	to->handle = data.handle;
#endif

#ifdef __SANITIZE_ADDRESS__
	__sanitizer_finish_switch_fiber(fake_stack, &to->stack->asan_pointer, &to->stack->asan_size);
#endif

	EG(current_fiber_context) = from;

	zend_fiber_restore_vm_state(&state);

	/* Destroy prior context if it has been marked as dead. */
	if (to->status == ZEND_FIBER_STATUS_DEAD) {
		zend_fiber_destroy_context(to);
	}
}

static void zend_fiber_cleanup(zend_fiber_context *context)
{
	zend_fiber *fiber = zend_fiber_from_context(context);

	zend_vm_stack current_stack = EG(vm_stack);
	EG(vm_stack) = fiber->vm_stack;
	zend_vm_stack_destroy();
	EG(vm_stack) = current_stack;
	fiber->execute_data = NULL;
	fiber->stack_bottom = NULL;
	fiber->caller = NULL;
}

static ZEND_STACK_ALIGNED void zend_fiber_execute(zend_fiber_transfer *transfer)
{
	ZEND_ASSERT(Z_TYPE(transfer->value) == IS_NULL && "Initial transfer value to fiber context must be NULL");
	ZEND_ASSERT(!transfer->flags && "No flags should be set on initial transfer");

	zend_fiber *fiber = EG(active_fiber);

	/* Determine the current error_reporting ini setting. */
	zend_long error_reporting = INI_INT("error_reporting");
	/* If error_reporting is 0 and not explicitly set to 0, INI_STR returns a null pointer. */
	if (!error_reporting && !INI_STR("error_reporting")) {
		error_reporting = E_ALL;
	}

	EG(vm_stack) = NULL;

	zend_first_try {
		zend_vm_stack stack = zend_vm_stack_new_page(ZEND_FIBER_VM_STACK_SIZE, NULL);
		EG(vm_stack) = stack;
		EG(vm_stack_top) = stack->top + ZEND_CALL_FRAME_SLOT;
		EG(vm_stack_end) = stack->end;
		EG(vm_stack_page_size) = ZEND_FIBER_VM_STACK_SIZE;

		fiber->execute_data = (zend_execute_data *) stack->top;
		fiber->stack_bottom = fiber->execute_data;

		memset(fiber->execute_data, 0, sizeof(zend_execute_data));

		fiber->execute_data->func = &zend_fiber_function;
		fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

		EG(current_execute_data) = fiber->execute_data;
		EG(jit_trace_num) = 0;
		EG(error_reporting) = error_reporting;

#ifdef ZEND_CHECK_STACK_LIMIT
		EG(stack_base) = zend_fiber_stack_base(fiber->context.stack);
		EG(stack_limit) = zend_fiber_stack_limit(fiber->context.stack);
#endif

		fiber->fci.retval = &fiber->result;

		zend_call_function(&fiber->fci, &fiber->fci_cache);

		//
		// PHP ASYNC FIBER FEATURES
		//
#ifdef PHP_ASYNC

		if (IS_ASYNC_ON) {
			async_fiber_shutdown_callback(fiber);
		}

        // Call shutdown handlers.
		if(fiber->shutdown_handlers) {
			zend_fiber_invoke_shutdown_handlers(fiber);
		}

		// Cleanup user local storage.
        if(fiber->fiber_storage) {
        	OBJ_RELEASE(&fiber->fiber_storage->std);
        	fiber->fiber_storage = NULL;
		}

#endif

		/* Cleanup callback and unset field to prevent GC / duplicate dtor issues. */
		zval_ptr_dtor(&fiber->fci.function_name);
		ZVAL_UNDEF(&fiber->fci.function_name);

		if (EG(exception)) {
			if (!(fiber->flags & ZEND_FIBER_FLAG_DESTROYED)
				|| !(zend_is_graceful_exit(EG(exception)) || zend_is_unwind_exit(EG(exception)))
			) {
				fiber->flags |= ZEND_FIBER_FLAG_THREW;
				transfer->flags = ZEND_FIBER_TRANSFER_FLAG_ERROR;

				ZVAL_OBJ_COPY(&transfer->value, EG(exception));
			}

			zend_clear_exception();
		}
	} zend_catch {
		fiber->flags |= ZEND_FIBER_FLAG_BAILOUT;
		transfer->flags = ZEND_FIBER_TRANSFER_FLAG_BAILOUT;
	} zend_end_try();

	fiber->context.cleanup = &zend_fiber_cleanup;
	fiber->vm_stack = EG(vm_stack);

	transfer->context = fiber->caller;
}

/* Handles forwarding of result / error from a transfer into the running fiber. */
static zend_always_inline void zend_fiber_delegate_transfer_result(
	zend_fiber_transfer *transfer, INTERNAL_FUNCTION_PARAMETERS
) {
	if (transfer->flags & ZEND_FIBER_TRANSFER_FLAG_ERROR) {
		/* Use internal throw to skip the Throwable-check that would fail for (graceful) exit. */
		zend_throw_exception_internal(Z_OBJ(transfer->value));
		RETURN_THROWS();
	}

	if (return_value != NULL) {
		RETURN_COPY_VALUE(&transfer->value);
	} else {
		zval_ptr_dtor(&transfer->value);
	}
}

static zend_always_inline zend_fiber_transfer zend_fiber_switch_to(
	zend_fiber_context *context, zval *value, bool exception
) {
	zend_fiber_transfer transfer = {
		.context = context,
		.flags = exception ? ZEND_FIBER_TRANSFER_FLAG_ERROR : 0,
	};

	if (value) {
		ZVAL_COPY(&transfer.value, value);
	} else {
		ZVAL_NULL(&transfer.value);
	}

	zend_fiber_switch_context(&transfer);

	/* Forward bailout into current fiber. */
	if (UNEXPECTED(transfer.flags & ZEND_FIBER_TRANSFER_FLAG_BAILOUT)) {
		EG(active_fiber) = NULL;
		zend_bailout();
	}

	return transfer;
}

static zend_always_inline zend_fiber_transfer zend_fiber_resume_internal(zend_fiber *fiber, zval *value, bool exception)
{
	zend_fiber *previous = EG(active_fiber);

	if (previous) {
		previous->execute_data = EG(current_execute_data);
	}

	fiber->caller = EG(current_fiber_context);
	EG(active_fiber) = fiber;

	zend_fiber_transfer transfer = zend_fiber_switch_to(fiber->previous, value, exception);

	EG(active_fiber) = previous;

	return transfer;
}

static zend_always_inline zend_fiber_transfer zend_fiber_suspend_internal(zend_fiber *fiber, zval *value)
{
	ZEND_ASSERT(!(fiber->flags & ZEND_FIBER_FLAG_DESTROYED));
	ZEND_ASSERT(fiber->context.status == ZEND_FIBER_STATUS_RUNNING || fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED);
	ZEND_ASSERT(fiber->caller != NULL);

	zend_fiber_context *caller = fiber->caller;
	fiber->previous = EG(current_fiber_context);
	fiber->caller = NULL;
	fiber->execute_data = EG(current_execute_data);

	return zend_fiber_switch_to(caller, value, false);
}

ZEND_API zend_result zend_fiber_start(zend_fiber *fiber, zval *return_value)
{
	ZEND_ASSERT(fiber->context.status == ZEND_FIBER_STATUS_INIT);

	if (zend_fiber_init_context(&fiber->context, zend_ce_fiber, zend_fiber_execute, EG(fiber_stack_size)) == FAILURE) {
		return FAILURE;
	}

	fiber->previous = &fiber->context;

	zend_fiber_transfer transfer = zend_fiber_resume_internal(fiber, NULL, false);

	zend_fiber_delegate_transfer_result(&transfer, EG(current_execute_data), return_value);

	return SUCCESS;
}

ZEND_API void zend_fiber_resume(zend_fiber *fiber, zval *value, zval *return_value)
{
	ZEND_ASSERT(fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED && fiber->caller == NULL);

	fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

	zend_fiber_transfer transfer = zend_fiber_resume_internal(fiber, value, /* exception */ false);

	zend_fiber_delegate_transfer_result(&transfer, EG(current_execute_data), return_value);
}

ZEND_API void zend_fiber_resume_exception(zend_fiber *fiber, zval *exception, zval *return_value)
{
	ZEND_ASSERT(fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED && fiber->caller == NULL);

	fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

	zend_fiber_transfer transfer = zend_fiber_resume_internal(fiber, exception, /* exception */ true);

	zend_fiber_delegate_transfer_result(&transfer, EG(current_execute_data), return_value);
}

ZEND_API void zend_fiber_suspend(zend_fiber *fiber, zval *value, zval *return_value)
{
	fiber->stack_bottom->prev_execute_data = NULL;

	zend_fiber_transfer transfer = zend_fiber_suspend_internal(fiber, value);

	zend_fiber_delegate_transfer_result(&transfer, EG(current_execute_data), return_value);
}

static zend_object *zend_fiber_object_create(zend_class_entry *ce)
{
	zend_fiber *fiber = emalloc(sizeof(zend_fiber));
	memset(fiber, 0, sizeof(zend_fiber));

	zend_object_std_init(&fiber->std, ce);
	return &fiber->std;
}

static void zend_fiber_object_destroy(zend_object *object)
{
	zend_fiber *fiber = (zend_fiber *) object;

	if (fiber->context.status != ZEND_FIBER_STATUS_SUSPENDED) {
		return;
	}

	zend_object *exception = EG(exception);
	EG(exception) = NULL;

	zval graceful_exit;
	ZVAL_OBJ(&graceful_exit, zend_create_graceful_exit());

	fiber->flags |= ZEND_FIBER_FLAG_DESTROYED;

	zend_fiber_transfer transfer = zend_fiber_resume_internal(fiber, &graceful_exit, true);

	zval_ptr_dtor(&graceful_exit);

	if (transfer.flags & ZEND_FIBER_TRANSFER_FLAG_ERROR) {
		EG(exception) = Z_OBJ(transfer.value);

		if (!exception && EG(current_execute_data) && EG(current_execute_data)->func
				&& ZEND_USER_CODE(EG(current_execute_data)->func->common.type)) {
			zend_rethrow_exception(EG(current_execute_data));
		}

		zend_exception_set_previous(EG(exception), exception);

		if (!EG(current_execute_data)) {
			zend_exception_error(EG(exception), E_ERROR);
		}
	} else {
		zval_ptr_dtor(&transfer.value);
		EG(exception) = exception;
	}
}

static void zend_fiber_object_free(zend_object *object)
{
	zend_fiber *fiber = (zend_fiber *) object;

	zval_ptr_dtor(&fiber->fci.function_name);
	zval_ptr_dtor(&fiber->result);

	zend_object_std_dtor(&fiber->std);
}

static HashTable *zend_fiber_object_gc(zend_object *object, zval **table, int *num)
{
	zend_fiber *fiber = (zend_fiber *) object;
	zend_get_gc_buffer *buf = zend_get_gc_buffer_create();

	zend_get_gc_buffer_add_zval(buf, &fiber->fci.function_name);
	zend_get_gc_buffer_add_zval(buf, &fiber->result);

	if (fiber->context.status != ZEND_FIBER_STATUS_SUSPENDED || fiber->caller != NULL) {
		zend_get_gc_buffer_use(buf, table, num);
		return NULL;
	}

	HashTable *lastSymTable = NULL;
	zend_execute_data *ex = fiber->execute_data;
	for (; ex; ex = ex->prev_execute_data) {
		HashTable *symTable;
		if (ZEND_CALL_INFO(ex) & ZEND_CALL_GENERATOR) {
			/* The generator object is stored in ex->return_value */
			zend_generator *generator = (zend_generator*)ex->return_value;
			/* There are two cases to consider:
			 * - If the generator is currently running, the Generator's GC
			 *   handler will ignore it because it is not collectable. However,
			 *   in this context the generator is suspended in Fiber::suspend()
			 *   and may be collectable, so we can inspect it.
			 * - If the generator is not running, the Generator's GC handler
			 *   will inspect it. In this case we have to skip the frame.
			 */
			if (!(generator->flags & ZEND_GENERATOR_CURRENTLY_RUNNING)) {
				continue;
			}
			symTable = zend_generator_frame_gc(buf, generator);
		} else {
			symTable = zend_unfinished_execution_gc_ex(ex, ex->func && ZEND_USER_CODE(ex->func->type) ? ex->call : NULL, buf, false);
		}
		if (symTable) {
			if (lastSymTable) {
				zval *val;
				ZEND_HASH_FOREACH_VAL(lastSymTable, val) {
					if (EXPECTED(Z_TYPE_P(val) == IS_INDIRECT)) {
						val = Z_INDIRECT_P(val);
					}
					zend_get_gc_buffer_add_zval(buf, val);
				} ZEND_HASH_FOREACH_END();
			}
			lastSymTable = symTable;
		}
	}

	zend_get_gc_buffer_use(buf, table, num);

	return lastSymTable;
}

ZEND_METHOD(Fiber, __construct)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_FUNC(fci, fcc)
	ZEND_PARSE_PARAMETERS_END();

	zend_fiber *fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(fiber->context.status != ZEND_FIBER_STATUS_INIT || Z_TYPE(fiber->fci.function_name) != IS_UNDEF)) {
		zend_throw_error(zend_ce_fiber_error, "Cannot call constructor twice");
		RETURN_THROWS();
	}

	fiber->fci = fci;
	fiber->fci_cache = fcc;

	// Keep a reference to closures or callable objects while the fiber is running.
	Z_TRY_ADDREF(fiber->fci.function_name);
}

ZEND_METHOD(Fiber, start)
{
	zend_fiber *fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	ZEND_PARSE_PARAMETERS_START(0, -1)
		Z_PARAM_VARIADIC_WITH_NAMED(fiber->fci.params, fiber->fci.param_count, fiber->fci.named_params);
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(zend_fiber_switch_blocked())) {
		zend_throw_error(zend_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	if (fiber->context.status != ZEND_FIBER_STATUS_INIT) {
		zend_throw_error(zend_ce_fiber_error, "Cannot start a fiber that has already been started");
		RETURN_THROWS();
	}

	if (zend_fiber_init_context(&fiber->context, zend_ce_fiber, zend_fiber_execute, EG(fiber_stack_size)) == FAILURE) {
		RETURN_THROWS();
	}

	fiber->previous = &fiber->context;

	zend_fiber_transfer transfer = zend_fiber_resume_internal(fiber, NULL, false);

	zend_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Fiber, suspend)
{
	zval *value = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(value);
	ZEND_PARSE_PARAMETERS_END();

	zend_fiber *fiber = EG(active_fiber);

	if (UNEXPECTED(!fiber)) {
		zend_throw_error(zend_ce_fiber_error, "Cannot suspend outside of a fiber");
		RETURN_THROWS();
	}

	if (UNEXPECTED(fiber->flags & ZEND_FIBER_FLAG_DESTROYED)) {
		zend_throw_error(zend_ce_fiber_error, "Cannot suspend in a force-closed fiber");
		RETURN_THROWS();
	}

	if (UNEXPECTED(zend_fiber_switch_blocked())) {
		zend_throw_error(zend_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	ZEND_ASSERT(fiber->context.status == ZEND_FIBER_STATUS_RUNNING || fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED);

	fiber->stack_bottom->prev_execute_data = NULL;

	zend_fiber_transfer transfer = zend_fiber_suspend_internal(fiber, value);

	zend_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Fiber, resume)
{
	zend_fiber *fiber;
	zval *value = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(value);
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(zend_fiber_switch_blocked())) {
		zend_throw_error(zend_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(fiber->context.status != ZEND_FIBER_STATUS_SUSPENDED || fiber->caller != NULL)) {
		zend_throw_error(zend_ce_fiber_error, "Cannot resume a fiber that is not suspended");
		RETURN_THROWS();
	}

	fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

	zend_fiber_transfer transfer = zend_fiber_resume_internal(fiber, value, false);

	zend_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Fiber, throw)
{
	zend_fiber *fiber;
	zval *exception;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(exception, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(zend_fiber_switch_blocked())) {
		zend_throw_error(zend_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(fiber->context.status != ZEND_FIBER_STATUS_SUSPENDED || fiber->caller != NULL)) {
		zend_throw_error(zend_ce_fiber_error, "Cannot resume a fiber that is not suspended");
		RETURN_THROWS();
	}

	fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

	zend_fiber_transfer transfer = zend_fiber_resume_internal(fiber, exception, true);

	zend_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Fiber, isStarted)
{
	zend_fiber *fiber;

	ZEND_PARSE_PARAMETERS_NONE();

	fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	RETURN_BOOL(fiber->context.status != ZEND_FIBER_STATUS_INIT);
}

ZEND_METHOD(Fiber, isSuspended)
{
	zend_fiber *fiber;

	ZEND_PARSE_PARAMETERS_NONE();

	fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	RETURN_BOOL(fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED && fiber->caller == NULL);
}

ZEND_METHOD(Fiber, isRunning)
{
	zend_fiber *fiber;

	ZEND_PARSE_PARAMETERS_NONE();

	fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	RETURN_BOOL(fiber->context.status == ZEND_FIBER_STATUS_RUNNING || fiber->caller != NULL);
}

ZEND_METHOD(Fiber, isTerminated)
{
	zend_fiber *fiber;

	ZEND_PARSE_PARAMETERS_NONE();

	fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	RETURN_BOOL(fiber->context.status == ZEND_FIBER_STATUS_DEAD);
}

ZEND_METHOD(Fiber, getReturn)
{
	zend_fiber *fiber;
	const char *message;

	ZEND_PARSE_PARAMETERS_NONE();

	fiber = (zend_fiber *) Z_OBJ_P(ZEND_THIS);

	if (fiber->context.status == ZEND_FIBER_STATUS_DEAD) {
		if (fiber->flags & ZEND_FIBER_FLAG_THREW) {
			message = "The fiber threw an exception";
		} else if (fiber->flags & ZEND_FIBER_FLAG_BAILOUT) {
			message = "The fiber exited with a fatal error";
		} else {
			RETURN_COPY_DEREF(&fiber->result);
		}
	} else if (fiber->context.status == ZEND_FIBER_STATUS_INIT) {
		message = "The fiber has not been started";
	} else {
		message = "The fiber has not returned";
	}

	zend_throw_error(zend_ce_fiber_error, "Cannot get fiber return value: %s", message);
	RETURN_THROWS();
}

ZEND_METHOD(Fiber, getCurrent)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zend_fiber *fiber = EG(active_fiber);

	if (!fiber) {
		RETURN_NULL();
	}

	RETURN_OBJ_COPY(&fiber->std);
}

//
// PHP ASYNC FIBER FEATURES
//
#ifdef PHP_ASYNC

static void shutdown_handlers_dtor(zval *zval_ptr)
{
	if (Z_TYPE_P(zval_ptr) == IS_PTR) {

	    zend_fiber_defer_entry *entry = Z_PTR_P(zval_ptr);

		if (entry != NULL && entry->object != NULL) {
			OBJ_RELEASE(entry->object);
			efree(entry);
		}

    } else {
	    zval_ptr_dtor(zval_ptr);
    }
}

zend_always_inline void shutdown_handlers_new(HashTable ** ht)
{
    ALLOC_HASHTABLE(*ht);
    zend_hash_init(*ht, 4, NULL, shutdown_handlers_dtor, 0);
}

ZEND_METHOD(Fiber, getContext)
{
	zend_fiber* fiber = (zend_fiber*)Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(fiber->fiber_storage == NULL)) {
		fiber->fiber_storage = zend_fiber_storage_new();
	}

	RETURN_OBJ_COPY(fiber->fiber_storage);
}

ZEND_METHOD(Fiber, defer)
{
	zval *callback;

	zend_fiber* fiber = (zend_fiber*)Z_OBJ_P(ZEND_THIS);

	ZEND_PARSE_PARAMETERS_START(1, 1)
	    Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	if (!zend_is_callable(callback, 0, NULL)) {
		zend_throw_exception_ex(zend_ce_type_error, 0, "Expected parameter \"callback\" should be a valid callable");
		RETURN_THROWS();
	}

	if (!fiber->shutdown_handlers) {
	    shutdown_handlers_new(&fiber->shutdown_handlers);
	}

	zval new_callback;
	ZVAL_COPY(&new_callback, callback);
	zend_hash_next_index_insert(fiber->shutdown_handlers, &new_callback);
}

ZEND_METHOD(Fiber, removeDeferHandler)
{
	zval *callback;

	const zend_fiber* fiber = (zend_fiber*)Z_OBJ_P(ZEND_THIS);

	ZEND_PARSE_PARAMETERS_START(1, 1)
	    Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	if (!fiber->shutdown_handlers) {
		RETURN_FALSE;
	}

	zend_ulong index;
	zend_string *key;
	zval *entry;

	ZEND_HASH_FOREACH_KEY_VAL(fiber->shutdown_handlers, index, key, entry) {
		if (zend_is_identical(entry, callback)) {
			zend_hash_index_del(fiber->shutdown_handlers, index);
			RETURN_TRUE;
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_FALSE;
}

#endif

#ifdef PHP_ASYNC

#define THIS_FIBER_CONTEXT ((zend_fiber_storage *)Z_OBJ_P(ZEND_THIS))

ZEND_METHOD(FiberContext, get)
{
	zend_string * key;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(key)
	ZEND_PARSE_PARAMETERS_END();

	zval * result = zend_hash_find(&THIS_FIBER_CONTEXT->storage, key);

	if (result == NULL) {
		RETURN_NULL();
	}

	RETURN_ZVAL(result, 1, 0);
}

ZEND_METHOD(FiberContext, has)
{
	zend_string * key;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(key)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL(zend_hash_find(&THIS_FIBER_CONTEXT->storage, key) != NULL);
}


ZEND_METHOD(FiberContext, set)
{
	zend_string * key;
	zval * value;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(key)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	zend_hash_update(&THIS_FIBER_CONTEXT->storage, key, value);
}

ZEND_METHOD(FiberContext, del)
{
	zend_string * key;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(key)
	ZEND_PARSE_PARAMETERS_END();

	zend_hash_del(&THIS_FIBER_CONTEXT->storage, key);
}

ZEND_METHOD(FiberContext, findObject)
{
	zend_string * type;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(type)
	ZEND_PARSE_PARAMETERS_END();

	zend_class_entry *ce = zend_lookup_class_ex(type, NULL, 0);

	if (ce == NULL) {
		zend_throw_error(zend_ce_type_error, "Type %s not found", ZSTR_VAL(type));
		RETURN_THROWS();
	}

	zend_object * result = zend_fiber_storage_find_object(THIS_FIBER_CONTEXT, ce);

	if (result == NULL) {
		RETURN_NULL();
	}

	RETURN_OBJ_COPY(result);
}

ZEND_METHOD(FiberContext, bindObject)
{
	zend_object * object;
	zend_string * type;
	zend_bool replace = 0;
	zend_class_entry *ce = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_OBJ(object)
		Z_PARAM_OPTIONAL
		Z_PARAM_STR(type)
		Z_PARAM_BOOL(replace)
	ZEND_PARSE_PARAMETERS_END();

	if (type == NULL) {
		ce = object->ce;
	} else {
		ce = zend_lookup_class(type);
	}

	if (ce == NULL) {
		zend_throw_error(zend_ce_type_error, "Type %s not found", ZSTR_VAL(type));
		RETURN_THROWS();
	}

	if (zend_fiber_storage_bind(THIS_FIBER_CONTEXT, object, ce, replace) == FAILURE) {
		zend_throw_error(NULL, "Failed to bind object to fiber context");
		RETURN_THROWS();
	}
}

ZEND_METHOD(FiberContext, unbindObject)
{
	zend_string * type;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(type)
	ZEND_PARSE_PARAMETERS_END();

	zend_class_entry *ce = zend_lookup_class_ex(type, NULL, 0);

	if (ce == NULL) {
		zend_throw_error(zend_ce_type_error, "Type %s not found", ZSTR_VAL(type));
		RETURN_THROWS();
	}

	zend_fiber_storage_unbind(THIS_FIBER_CONTEXT, ce);
}

#endif

ZEND_METHOD(FiberError, __construct)
{
	zend_throw_error(
		NULL,
		"The \"%s\" class is reserved for internal use and cannot be manually instantiated",
		ZSTR_VAL(Z_OBJCE_P(ZEND_THIS)->name)
	);
}


void zend_register_fiber_ce(void)
{
	zend_ce_fiber = register_class_Fiber();
	zend_ce_fiber->create_object = zend_fiber_object_create;
	zend_ce_fiber->default_object_handlers = &zend_fiber_handlers;

	zend_fiber_handlers = std_object_handlers;
	zend_fiber_handlers.dtor_obj = zend_fiber_object_destroy;
	zend_fiber_handlers.free_obj = zend_fiber_object_free;
	zend_fiber_handlers.get_gc = zend_fiber_object_gc;
	zend_fiber_handlers.clone_obj = NULL;

	zend_ce_fiber_error = register_class_FiberError(zend_ce_error);
	zend_ce_fiber_error->create_object = zend_ce_error->create_object;
#ifdef PHP_ASYNC
	zend_ce_fiber_context = register_class_FiberContext();
#endif
}

void zend_fiber_init(void)
{
	zend_fiber_context *context = ecalloc(1, sizeof(zend_fiber_context));

#if defined(__SANITIZE_ADDRESS__) || defined(ZEND_FIBER_UCONTEXT)
	// Main fiber stack is only needed if ASan or ucontext is enabled.
	context->stack = emalloc(sizeof(zend_fiber_stack));

#ifdef ZEND_FIBER_UCONTEXT
	context->handle = &context->stack->ucontext;
#endif
#endif

	context->status = ZEND_FIBER_STATUS_RUNNING;

	EG(main_fiber_context) = context;
	EG(current_fiber_context) = context;
	EG(active_fiber) = NULL;

	zend_fiber_switch_blocking = 0;
}

void zend_fiber_shutdown(void)
{
#if defined(__SANITIZE_ADDRESS__) || defined(ZEND_FIBER_UCONTEXT)
	efree(EG(main_fiber_context)->stack);
#endif

	efree(EG(main_fiber_context));

	zend_fiber_switch_block();
}

#ifdef PHP_ASYNC

void zend_fiber_defer(zend_fiber *fiber, const zend_fiber_defer_entry * entry)
{
	if (fiber->shutdown_handlers == NULL) {
		shutdown_handlers_new(&fiber->shutdown_handlers);
	}

	zval z_entry;
	ZVAL_PTR(&z_entry, entry);

	if (zend_hash_next_index_insert(fiber->shutdown_handlers, &z_entry) != NULL) {
        GC_ADDREF(entry->object);
    }
}

zend_fiber_storage * zend_fiber_storage_new(void)
{
	const size_t size = sizeof(zend_fiber_storage) + zend_object_properties_size(zend_ce_fiber_context);
	zend_fiber_storage *object = emalloc(size);
	memset(object, 0, size);

	zend_hash_init(&object->storage, 0, NULL, ZVAL_PTR_DTOR, 0);

	return object;
}

zend_object * zend_fiber_storage_find_object(const zend_fiber_storage *storage, zend_class_entry * class_entry)
{
	zval *value = zend_hash_index_find(&storage->storage, (zend_ulong) class_entry);

	if (value == NULL) {
		return NULL;
	}

	if (Z_TYPE_P(value) != IS_OBJECT) {
		return NULL;
	}

	return Z_OBJ_P(value);
}

zend_result zend_fiber_storage_bind(zend_fiber_storage *storage, zend_object *object, zend_class_entry * class_entry, bool replace)
{
	if (class_entry == NULL) {
		class_entry = object->ce;
	}

	// Class entry point as array int key.
	zval *entry = zend_hash_index_find(&storage->storage, (zend_ulong) class_entry);

	if (entry != NULL && replace == false) {
		return FAILURE;
	}

	zval z_object;
	ZVAL_OBJ(&z_object, object);
	if (zend_hash_index_update(&storage->storage, (zend_ulong) class_entry, &z_object) == NULL) {
		return FAILURE;
	}

	GC_ADDREF(object);
	return SUCCESS;
}

void zend_fiber_storage_unbind(zend_fiber_storage *storage, zend_class_entry * class_entry)
{
	zend_hash_index_del(&storage->storage, (zend_ulong) class_entry);
}

#endif
