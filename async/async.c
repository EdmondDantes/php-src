
#include "php_async.h"

// ===========================
// DeferredResume class
// ===========================
// region DeferredResume

ZEND_METHOD(Async_DeferredResume, __construct)
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

void zend_fiber_shutdown(void)
{
#if defined(__SANITIZE_ADDRESS__) || defined(ZEND_FIBER_UCONTEXT)
	efree(EG(main_fiber_context)->stack);
#endif

	efree(EG(main_fiber_context));

	zend_fiber_switch_block();
}

static zend_object *async_deferred_resume_object_create(zend_class_entry *ce)
{
	async_deferred_resume *deferred_resume = emalloc(sizeof(async_deferred_resume));
	memset(deferred_resume, 0, sizeof(async_deferred_resume));

	zend_object_std_init(&deferred_resume->std, ce);
	return &deferred_resume->std;
}

static void async_deferred_resume_destroy(zend_object *object)
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

static void async_deferred_resume_free(zend_object *object)
{
	async_deferred_resume *deferred_resume = (async_deferred_resume *) object;

	zval_ptr_dtor(&deferred_resume->fci.function_name);
	zval_ptr_dtor(&deferred_resume->result);

	zend_object_std_dtor(&deferred_resume->std);
}

static zend_object_handlers async_deferred_resume_handlers;

void zend_register_deferred_resume_ce(void)
{
	zend_ce_deferred_resume 					= register_class_Async_DeferredResume();
	zend_ce_deferred_resume->create_object 		= async_deferred_resume_object_create;
	zend_ce_deferred_resume->default_object_handlers = &async_deferred_resume_handlers;

	async_deferred_resume_handlers 				= std_object_handlers;
	async_deferred_resume_handlers.dtor_obj 	= async_deferred_resume_destroy;
	async_deferred_resume_handlers.free_obj 	= async_deferred_resume_free;
	async_deferred_resume_handlers.clone_obj 	= NULL;
}

// endregion
// ===========================
// DeferredResume Section
// ===========================