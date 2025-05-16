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
#include "scope.h"
#include "scope_arginfo.h"
#include "zend_common.h"

static zend_object_handlers async_scope_handlers;

static zend_always_inline void
async_scope_add_coroutine(async_scope_t *scope, async_coroutine_t *coroutine)
{
	async_coroutines_vector_t *vector = &scope->coroutines;

	if (vector->data == NULL) {
		vector->data = safe_emalloc(4, sizeof(async_coroutine_t *), 0);
		vector->capacity = 4;
	}

	if (vector->length == vector->capacity) {
		vector->capacity *= 2;
		vector->data = safe_erealloc(vector->data, vector->capacity, sizeof(async_coroutine_t *), 0);
	}

	vector->data[vector->length++] = coroutine;
}

static zend_always_inline void
async_scope_remove_coroutine(async_scope_t *scope, async_coroutine_t *coroutine)
{
	async_coroutines_vector_t *vector = &scope->coroutines;
	for (uint32_t i = 0; i < vector->length; ++i) {
		if (vector->data[i] == coroutine) {
			vector->data[i] = vector->data[--vector->length];
			return;
		}
	}
}

static zend_always_inline void
async_scope_free_coroutines(async_scope_t *scope)
{
	async_coroutines_vector_t *vector = &scope->coroutines;

	if (vector->data != NULL) {
		efree(vector->data);
	}

	vector->data = NULL;
	vector->length = 0;
	vector->capacity = 0;
}

static void scope_before_coroutine_enqueue(zend_async_scope_t *zend_scope, zend_coroutine_t *coroutine)
{
	async_scope_t *scope = (async_scope_t *) zend_scope;

	async_scope_add_coroutine(scope, (async_coroutine_t *) coroutine);
}

static void scope_after_coroutine_enqueue(zend_async_scope_t *scope, zend_coroutine_t *coroutine)
{
}

static void scope_dispose(zend_async_scope_t *zend_scope)
{
	async_scope_t *scope = (async_scope_t *) zend_scope;
	async_scope_free_coroutines(scope);
}

zend_async_scope_t * async_new_scope(zend_async_scope_t * parent_scope)
{
	DEFINE_ZEND_INTERNAL_OBJECT(async_scope_t, scope, async_ce_scope);

	if (UNEXPECTED(EG(exception))) {
		return NULL;
	}

	scope->scope.parent_scope = parent_scope;
	ZEND_ASYNC_SCOPE_SET_ZEND_OBJ(&scope->scope);
	ZEND_ASYNC_SCOPE_SET_NO_FREE_MEMORY(&scope->scope);
	ZEND_ASYNC_SCOPE_SET_ZEND_OBJ_OFFSET(&scope->scope, XtOffsetOf(async_scope_t, std));

	scope->scope.before_coroutine_enqueue = scope_before_coroutine_enqueue;
	scope->scope.after_coroutine_enqueue = scope_after_coroutine_enqueue;
	scope->scope.dispose = scope_dispose;

	return &scope->scope;
}

zend_object *scope_object_create(zend_class_entry *class_entry)
{
	async_scope_t *scope = (async_scope_t *) zend_object_alloc(
		sizeof(async_scope_t) + zend_object_properties_size(async_ce_scope), class_entry
	);

	zend_object_std_init(&scope->std, class_entry);
	object_properties_init(&scope->std, class_entry);

	if (UNEXPECTED(EG(exception))) {
		return NULL;
	}

	ZEND_ASYNC_SCOPE_SET_ZEND_OBJ(&scope->scope);
	ZEND_ASYNC_SCOPE_SET_NO_FREE_MEMORY(&scope->scope);
	ZEND_ASYNC_SCOPE_SET_ZEND_OBJ_OFFSET(&scope->scope, XtOffsetOf(async_scope_t, std));

	scope->scope.before_coroutine_enqueue = scope_before_coroutine_enqueue;
	scope->scope.after_coroutine_enqueue = scope_after_coroutine_enqueue;
	scope->scope.dispose = scope_dispose;

	return &scope->std;
}

static void scope_destroy(zend_object *object)
{
	async_scope_t *scope = (async_scope_t *) object;

	if (scope->scope.parent_scope) {
		zend_async_scope_remove_child(scope->scope.parent_scope, &scope->scope);
	}

	scope->scope.before_coroutine_enqueue = NULL;
	scope->scope.after_coroutine_enqueue = NULL;
	scope->scope.dispose = NULL;

	zend_object_std_dtor(&scope->std);
}

static void scope_free(zend_object *object)
{
	async_scope_t *scope = (async_scope_t *) object;

	async_scope_free_coroutines(scope);

	if (scope->scope.parent_scope) {
		zend_async_scope_remove_child(scope->scope.parent_scope, &scope->scope);
	}

	zend_object_std_dtor(&scope->std);
}

void async_register_scope_ce(void)
{
	async_ce_scope_provider = register_class_Async_ScopeProvider();
	async_ce_spawn_strategy = register_class_Async_SpawnStrategy(async_ce_scope_provider);
	async_ce_scope = register_class_Async_Scope(async_ce_scope_provider);

	async_ce_scope->create_object = scope_object_create;

	async_scope_handlers = std_object_handlers;

	async_scope_handlers.offset   = XtOffsetOf(async_scope_t, std);
	async_scope_handlers.clone_obj = NULL;
	async_scope_handlers.dtor_obj = scope_destroy;
	async_scope_handlers.free_obj = scope_free;

}