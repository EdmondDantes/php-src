//
// Created by Edmond on 16.12.2024.
//

#ifndef ASYNC_CALLBACK_H
#define ASYNC_CALLBACK_H

ZEND_API zend_class_entry *async_ce_callback;

void async_callback_notify(zend_object *callback, zend_object *notifier, const zval *event, const zval *error);
zend_result async_callback_bind_resume(zend_object* callback, const zval* resume);
void async_callback_registered(zend_object* callback, const zval* notifier);
zval* async_callback_get_resume(const zend_object* callback);

#endif //ASYNC_CALLBACK_H
