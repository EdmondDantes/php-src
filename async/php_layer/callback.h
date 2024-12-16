//
// Created by Edmond on 16.12.2024.
//

#ifndef CALLBACK_H
#define CALLBACK_H

zend_result async_callback_notify(zend_object *callback, zend_object *notifier, zval *event, zval *error);

#endif //CALLBACK_H
