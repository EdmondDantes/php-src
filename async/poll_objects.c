//
// Created by Edmond on 07.12.2024.
//

#include "poll_objects.h"
#include <threads.h>

thread_local HashTable *fd_to_fiber = NULL;

static HashTable * get_fd_to_fiber()
{
    if (fd_to_fiber == NULL) {
        ALLOC_HASHTABLE(fd_to_fiber);
        zend_hash_init(fd_to_fiber, 32, NULL, NULL, 0);
    }

    return fd_to_fiber;
}

void async_new_poll_object()
{

}

void * async_bind_fd_to_fiber(zend_fiber *fiber, int fd)
{
    return zend_hash_index_add_mem(get_fd_to_fiber(), fd, &fiber, sizeof(void *));
}

void async_unbind_fd_from_fiber(int fd)
{
    zend_hash_index_del(get_fd_to_fiber(), fd);
}

zend_fiber * async_get_fd_fiber_binding(int fd)
{
    zend_fiber **found_pointer = (zend_fiber **) zend_hash_index_find(get_fd_to_fiber(), fd);
    if (found_pointer) {
        return *found_pointer;
    } else {
        return NULL;
    }
}