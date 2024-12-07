
#include "network.h"

/* Connect to a socket using an interruptible connect with optional timeout.
 * Optionally, the connect can be made asynchronously, which will implicitly
 * enable non-blocking mode on the socket.
 * */
/* {{{ php_async_network_connect_socket */
PHPAPI int php_async_network_connect_socket(php_socket_t sockfd,
		const struct sockaddr *addr,
		socklen_t addrlen,
		int asynchronous,
		struct timeval *timeout,
		zend_string **error_string,
		int *error_code)
{

}
/* }}} */

/* Accept a client connection from a server socket,
 * using an optional timeout.
 * Returns the peer address in addr/addrlen (it will emalloc
 * these, so be sure to efree the result).
 * If you specify textaddr, a text-printable
 * version of the address will be emalloc'd and returned.
 * */

/* {{{ php_async_network_accept_incoming */
PHPAPI php_socket_t php_async_network_accept_incoming(php_socket_t srvsock,
		zend_string **textaddr,
		struct sockaddr **addr,
		socklen_t *addrlen,
		struct timeval *timeout,
		zend_string **error_string,
		int *error_code,
		int tcp_nodelay
		)
{
}
/* }}} */