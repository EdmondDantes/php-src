//
// Created by Edmond on 16.12.2024.
//

#ifndef CALLBACK_H
#define CALLBACK_H

ZEND_API zend_class_entry *async_ce_callback;

void async_callback_notify(zend_object *callback, zend_object *notifier, zval *event, zval *error);
zend_result async_callback_bind_resume(zend_object* callback, const zval* resume);
void async_callback_registered(zend_object* callback, const zval* notifier);
zval* async_callback_get_resume(const zend_object* callback);

#endif //CALLBACK_H
