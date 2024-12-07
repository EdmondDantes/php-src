//
//
//

#ifndef NETWORK_H
#define NETWORK_H

#include "php.h"

#include <stddef.h>
#include <errno.h>
#include "main/php_network.h"


#ifdef PHP_WIN32
# include <Ws2tcpip.h>
# include "win32/winutil.h"
# define O_RDONLY _O_RDONLY
# include "win32/param.h"
#else
#include <sys/param.h>
#endif


PHPAPI int php_async_network_connect_socket(php_socket_t sockfd,
        const struct sockaddr *addr,
        socklen_t addrlen,
        int asynchronous,
        struct timeval *timeout,
        zend_string **error_string,
        int *error_code);

#endif //NETWORK_H
