//
// Created by Edmond on 07.12.2024.
//

#ifndef ZEND_ENGINE_H
#define ZEND_ENGINE_H

#include <Zend/zend.h>

#define INVALID_POINTER ((void *)-1)

#define THIS_PROPERTY(prop_index) OBJ_PROP_NUM(Z_OBJ_P(ZEND_THIS), (prop_index))

#define GET_CLASS_ENTRY(class_name_str, ce_ptr) static zend_class_entry *(ce_ptr) = INVALID_POINTER; 		\
    if ((ce_ptr) == INVALID_POINTER) {                                                		\
        zend_string *class_name = zend_string_init((class_name_str), 						\
                                                   strlen(class_name_str), 0); 				\
        (ce_ptr) = zend_hash_find_ptr(EG(class_table), class_name); 						\
        zend_string_release(class_name);                             						\
    }


#endif //ZEND_ENGINE_H
