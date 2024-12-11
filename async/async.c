
#include "php_async.h"

static zval* weak_reference_from(zval* referent) {

	static zend_class_entry* weak_ref_ce = NULL;
	static zval method_name;
	static int initialized = 0;

	zval* retval = emalloc(sizeof(zval));
	ZVAL_UNDEF(retval);

	if (!initialized) {
		weak_ref_ce = zend_lookup_class(ZEND_STRL("WeakReference"));
		if (!weak_ref_ce) {
			zend_throw_error(NULL, "Class WeakReference not found");
			efree(retval);
			return NULL;
		}
		ZVAL_STRING(&method_name, "create");
		initialized = 1;
	}

	if (call_user_function(
		NULL, NULL,
		&method_name,
		retval, 1, referent
	) != SUCCESS) {
		zend_throw_error(NULL, "Failed to call WeakReference::create");
		efree(retval);
		return NULL;
	}

	return retval;
}


// ===========================
#pragma region CompletionPublisher class
// CompletionPublisher class
// ===========================

static zend_result async_completion_add_callback(async_completion_publisher_t* publisher, zval* callback, ASYNC_COMPLETION_ACTION action) {

	if (!zend_is_callable(callback, 0, NULL)) {
		zend_throw_exception_ex(zend_ce_type_error, 0, "Argument is not a valid callable");
		return FAILURE;
	}	

	if (publisher->bindedObjects == NULL) {
		ALLOC_HASHTABLE(publisher->bindedObjects);
		zend_hash_init(publisher->bindedObjects, 0, NULL, ZVAL_PTR_DTOR, 0);
	}

	// Allocate async_completion_binded_object_t structure
	async_completion_binded_object_t* binded_obj = emalloc(sizeof(async_completion_binded_object_t));
	binded_obj->weak_reference = weak_reference_from(callback);
	binded_obj->action = ASYNC_COMPLETION_RESOLV;

	zend_hash_next_index_insert(publisher->bindedObjects, binded_obj);

	return SUCCESS;
}

// ---------------------------
#pragma region CompletionPublisher class methods
// CompletionPublisher class methods start
// ---------------------------

ZEND_METHOD(Async_CompletionPublisher, onSuccess)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	async_completion_publisher_t* publisher = (async_completion_publisher_t*)Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(async_completion_add_callback(publisher, callback, ASYNC_COMPLETION_RESOLV) != SUCCESS)) {
		RETURN_THROWS();
	}

	RETURN_OBJ(Z_OBJ_P(ZEND_THIS));
}

ZEND_METHOD(Async_CompletionPublisher, onError)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	async_completion_publisher_t* publisher = (async_completion_publisher_t*)Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(async_completion_add_callback(publisher, callback, ASYNC_COMPLETION_REJECT) != SUCCESS)) {
		RETURN_THROWS();
	}

	RETURN_OBJ(Z_OBJ_P(ZEND_THIS));
}

ZEND_METHOD(Async_CompletionPublisher, onFinally)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	async_completion_publisher_t* publisher = (async_completion_publisher_t*)Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(async_completion_add_callback(publisher, callback, ASYNC_COMPLETION_FINALLY) != SUCCESS)) {
		RETURN_THROWS();
	}

	RETURN_OBJ(Z_OBJ_P(ZEND_THIS));
}

ZEND_METHOD(Async_CompletionPublisher, thenCancel)
{
	zval* objects;
	int object_count, i;

	// Разбираем все переданные аргументы как массив zval
	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_VARIADIC('+', objects, object_count)
		ZEND_PARSE_PARAMETERS_END();

	// Обрабатываем переданные объекты
	for (i = 0; i < object_count; i++) {
		if (Z_TYPE(objects[i]) != IS_OBJECT || !instanceof_function(Z_OBJCE(objects[i]), CancellationInterface_ce)) {
			zend_throw_exception_ex(zend_ce_type_error, 0,
				"All arguments must implement CancellationInterface");
			RETURN_THROWS();
		}

		// Здесь можно сохранить объект в свойство или массив
		// Пример: добавить объект в свойство-список
		add_next_index_zval(zend_read_property(MyClass_ce, Z_OBJ_P(ZEND_THIS), "cancellations", sizeof("cancellations") - 1, 1, NULL), &objects[i]);

		// Увеличиваем ссылочный счётчик, так как zval добавляется в массив
		Z_TRY_ADDREF(objects[i]);
	}

	async_completion_publisher_t* publisher = (async_completion_publisher_t*)Z_OBJ_P(ZEND_THIS);

	if (UNEXPECTED(async_completion_add_callback(publisher, callback, ASYNC_COMPLETION_FINALLY) != SUCCESS)) {
		RETURN_THROWS();
	}

	RETURN_OBJ(Z_OBJ_P(ZEND_THIS));
}


// ---------------------------
// CompletionPublisher class methods end
#pragma endregion
// ---------------------------

// ---------------------------
#pragma region CompletionPublisher class zend handlers
// ---------------------------

static zend_object* async_completion_publisher_object_create(zend_class_entry* ce)
{
	async_completion_publisher_t* publisher = emalloc(sizeof(async_completion_publisher_t));
	memset(publisher, 0, sizeof(async_completion_publisher_t));

	zend_object_std_init(&publisher->std, ce);

	// Initialize the HashTable
	ALLOC_HASHTABLE(publisher->bindedObjects);
	zend_hash_init(publisher->bindedObjects, 0, NULL, ZVAL_PTR_DTOR, 0);

	return &publisher->std;
}

static void async_completion_publisher_free(zend_object* object)
{
	async_completion_publisher_t* publisher = (async_completion_publisher_t*)object;

	if (EXPECTED(publisher->bindedObjects)) {
		struct _async_completion_binded_object_s* binded_obj;
		zval* item;

		ZEND_HASH_FOREACH_VAL(publisher->bindedObjects, item) {
			binded_obj = Z_PTR_P(item);

			if (EXPECTED(binded_obj->weak_reference)) {
				zval_ptr_dtor(binded_obj->weak_reference);
				efree(binded_obj->weak_reference);
			}

			efree(binded_obj);
		} ZEND_HASH_FOREACH_END();

		zend_hash_destroy(publisher->bindedObjects);
		FREE_HASHTABLE(publisher->bindedObjects);
	}

	zend_object_std_dtor(&publisher->std);
}

static zend_object_handlers async_completion_publisher_handlers;

void async_register_completion_publisher_ce(void)
{
	zend_ce_completion_publisher = register_class_Async_CompletionPublisher();
	zend_ce_completion_publisher->create_object = async_completion_publisher_object_create;
	zend_ce_completion_publisher->default_object_handlers = &async_completion_publisher_handlers;

	async_completion_publisher_handlers = std_object_handlers;
	async_completion_publisher_handlers.free_obj = async_completion_publisher_free;
	async_completion_publisher_handlers.clone_obj = NULL;
}

// ---------------------------
// CompletionPublisher class zend handlers end
#pragma endregion
// ---------------------------

// ===========================
// CompletionPublisher class end
#pragma endregion
// ===========================


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
	ZEND_PARSE_PARAMETERS_NONE();

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
