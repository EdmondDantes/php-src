//
// Created by Edmond on 07.12.2024.
//

#ifndef ZEND_API_MACROS_H
#define ZEND_API_MACROS_H

#include <Zend/zend.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_globals.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_list.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>

#define INVALID_POINTER ((void *)-1)

#define GET_CLASS_ENTRY(class_name_str, ce_ptr) static zend_class_entry *(ce_ptr) = INVALID_POINTER; 		\
    if ((ce_ptr) == INVALID_POINTER) {                                                		\
        zend_string *class_name = zend_string_init((class_name_str), 						\
                                                   strlen(class_name_str), 0); 				\
        (ce_ptr) = zend_hash_find_ptr(EG(class_table), class_name); 						\
        zend_string_release(class_name);                             						\
    }


#endif //ZEND_API_MACROS_H
