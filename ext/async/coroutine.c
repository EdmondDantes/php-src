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
#include "coroutine.h"
#include "coroutine_arginfo.h"
#include "php_async.h"

#include "php_scheduler.h"
#include "zend_common.h"
#include "zend_exceptions.h"
#include "zend_ini.h"

static zend_function coroutine_root_function = { ZEND_INTERNAL_FUNCTION };

static void async_coroutine_cleanup(zend_fiber_context *context)
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

ZEND_STACK_ALIGNED void async_coroutine_execute(zend_fiber_transfer *transfer)
{
	ZEND_ASSERT(Z_TYPE(transfer->value) == IS_NULL && "Initial transfer value to coroutine context must be NULL");
	ZEND_ASSERT(!transfer->flags && "No flags should be set on initial transfer");

	async_coroutine_t *coroutine = (async_coroutine_t *) EG(coroutine);

	/* Determine the current error_reporting ini setting. */
	zend_long error_reporting = INI_INT("error_reporting");
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

		coroutine->execute_data = (zend_execute_data *) stack->top;

		memset(coroutine->execute_data, 0, sizeof(zend_execute_data));

		coroutine->execute_data->func = &coroutine_root_function;

		EG(current_execute_data) = coroutine->execute_data;
		EG(jit_trace_num) = 0;
		EG(error_reporting) = (int) error_reporting;

#ifdef ZEND_CHECK_STACK_LIMIT
		EG(stack_base) = zend_fiber_stack_base(coroutine->context.stack);
		EG(stack_limit) = zend_fiber_stack_limit(coroutine->context.stack);
#endif

		if (EXPECTED(coroutine->coroutine.internal_function == NULL))
		{
			coroutine->coroutine.fcall->fci.retval = &coroutine->coroutine.result;

			zend_call_function(&coroutine->coroutine.fcall->fci, &coroutine->coroutine.fcall->fci_cache);

			zval_ptr_dtor(&coroutine->coroutine.fcall->fci.function_name);
			ZVAL_UNDEF(&coroutine->coroutine.fcall->fci.function_name);
		} else {
			coroutine->coroutine.internal_function();
		}

		if (EG(exception)) {
			if (!(coroutine->flags & ZEND_FIBER_FLAG_DESTROYED)
				|| !(zend_is_graceful_exit(EG(exception)) || zend_is_unwind_exit(EG(exception)))
			) {
				coroutine->flags |= ZEND_FIBER_FLAG_THREW;
				transfer->flags = ZEND_FIBER_TRANSFER_FLAG_ERROR;

				ZVAL_OBJ_COPY(&transfer->value, EG(exception));
			}

			zend_clear_exception();
		}
	} zend_catch {
		coroutine->flags |= ZEND_FIBER_FLAG_BAILOUT;
		transfer->flags = ZEND_FIBER_TRANSFER_FLAG_BAILOUT;
	} zend_end_try();

	coroutine->context.cleanup = &async_coroutine_cleanup;
	coroutine->vm_stack = EG(vm_stack);

	transfer->context = NULL;

	async_scheduler_coroutine_suspend(transfer);
}

static void async_coroutine_dispose(zend_coroutine_t *coroutine)
{

}

static zend_object *async_coroutine_object_create(zend_class_entry *class_entry)
{
	async_coroutine_t *coroutine = pecalloc(1, sizeof(async_coroutine_t), 0);

	ZVAL_UNDEF(&coroutine->coroutine.result);

	coroutine->coroutine.dispose = async_coroutine_dispose;
	coroutine->flags = ZEND_FIBER_STATUS_INIT;

	zend_object_std_init(&coroutine->std, class_entry);

	return &coroutine->std;
}

static zend_object_handlers coroutine_handlers;

void async_register_coroutine_ce(void)
{
	async_ce_coroutine = register_class_Async_Coroutine(async_ce_awaitable);

	coroutine_handlers = std_object_handlers;
	coroutine_handlers.dtor_obj = async_coroutine_object_destroy;
	coroutine_handlers.clone_obj = async_coroutine_clone;
}