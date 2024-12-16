//
// Created by Edmond on 16.12.2024.
//

#ifndef ZEND_COMMON_H
#define ZEND_COMMON_H

#include "php.h"
#include "zend_exceptions.h"
#include "zend_smart_str.h"
#include "zend_interfaces.h"

#define IF_THROW_RETURN_VOID if(EG(exception) != NULL) { return; }
#define IF_THROW_RETURN(value) if(EG(exception) != NULL) { return value; }

zval* async_new_weak_reference_from(zval* referent);

#endif //ZEND_COMMON_H
