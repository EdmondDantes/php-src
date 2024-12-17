//
// Created by Edmond on 16.12.2024.
//

#ifndef CALLBACK_H
#define CALLBACK_H

ZEND_API zend_class_entry *async_ce_callback;

zend_result async_callback_notify(zend_object *callback, zend_object *notifier, zval *event, zval *error);

#endif //CALLBACK_H
