
#include "php_async.h"

// ===========================
#pragma region DeferredResume class
// DeferredResume class
// ===========================

// ---------------------------
#pragma region DeferredResume class methods
// DeferredResume class methods start
// ---------------------------

ZEND_METHOD(Async_DeferredResume, __construct)
{
	if (ZEND_NUM_ARGS() > 0) {
		php_error_docref(NULL, E_WARNING, "DeferredResume сonstructor does not expect any parameters, but %d were provided", ZEND_NUM_ARGS());
	}

	async_deferred_resume_t *deferred_resume = (async_deferred_resume_t *) Z_OBJ_P(ZEND_THIS);

	zend_fiber* current_fiber = EG(current_fiber);

	if (UNEXPECTED(current_fiber != NULL && current_fiber->context.status != ZEND_FIBER_STATUS_RUNNING)) {
		zend_throw_error(zend_ce_fiber_error, "Attempt to create a DeferredResume for a Fiber that is not running");
		RETURN_THROWS();
	}

	deferred_resume->fiber = current_fiber;
}

// ---------------------------
// DeferredResume class methods end
#pragma endregion
// ---------------------------


// ---------------------------
#pragma region DeferredResume class zend handlers
// ---------------------------

static zend_object *async_deferred_resume_object_create(zend_class_entry *ce)
{
	async_deferred_resume_t *deferred_resume = emalloc(sizeof(async_deferred_resume_t));
	memset(deferred_resume, 0, sizeof(async_deferred_resume_t));

	zend_object_std_init(&deferred_resume->std, ce);
	return &deferred_resume->std;
}

static void async_deferred_resume_destroy(zend_object *object)
{
	async_deferred_resume_t * deferred_resume = (async_deferred_resume_t*) object;
}

static void async_deferred_resume_free(zend_object *object)
{
	async_deferred_resume_t *deferred_resume = (async_deferred_resume_t *) object;
	zend_object_std_dtor(&deferred_resume->std);
}

static zend_object_handlers async_deferred_resume_handlers;

void void async_register_deferred_resume_ce(void)
{
	zend_ce_deferred_resume 					= register_class_Async_DeferredResume();
	zend_ce_deferred_resume->create_object 		= async_deferred_resume_object_create;
	zend_ce_deferred_resume->default_object_handlers = &async_deferred_resume_handlers;

	async_deferred_resume_handlers 				= std_object_handlers;
	async_deferred_resume_handlers.dtor_obj 	= async_deferred_resume_destroy;
	async_deferred_resume_handlers.free_obj 	= async_deferred_resume_free;
	async_deferred_resume_handlers.clone_obj 	= NULL;

}

// ---------------------------
// DeferredResume class zend handlers end
#pragma endregion
// ---------------------------

// ===========================
// DeferredResume class end
#pragma endregion
// ===========================
