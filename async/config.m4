PHP_ARG_ENABLE([async],
  [  --enable-async          Enable True Async API],
  [no])

PHP_ARG_WITH([async_libuv],
  [  --with-async-libuv      Build True Async API with libuv support],
  [no])

if test "$PHP_ASYNC" = "yes"; then
  dnl Define a symbol for C code.
  AC_DEFINE([HAVE_ASYNC], 1, [Enable True Async API])

  dnl Register extension source files.
  PHP_NEW_EXTENSION(
    async,
    async/async.c async/reactor.c async/scheduler.c \
    async/internal/allocator.c async/internal/circular_buffer.c \
    async/php_layer/callback.c async/php_layer/channel.c \
    async/php_layer/reactor_handles.c async/php_layer/exceptions.c \
    async/php_layer/module_entry.c async/php_layer/notifier.c \
    async/php_layer/resume.c async/php_layer/zend_common.c,
    $ext_shared
  )

  dnl Optionally install headers (if desired for public use).
  PHP_INSTALL_HEADERS([async], [php_async.h php_reactor.h php_scheduler.h])
  PHP_INSTALL_HEADERS([async/libuv], [libuv_reactor.h])
  PHP_INSTALL_HEADERS([async/php_layer], [
    callback.h channel.h reactor_handles.h exceptions.h
    module_entry.h notifier.h resume.h
  ])

  if test "$PHP_ASYNC_LIBUV" = "yes"; then
    if test "$PHP_ASYNC" != "yes"; then
      AC_MSG_ERROR([libuv requires True Async API. Use --enable-async.])
    fi

    dnl Check for libuv/uv.h
    AC_CHECK_HEADER([libuv/uv.h], [have_uv_h=yes], [have_uv_h=no])
    if test "$have_uv_h" = "no"; then
      AC_MSG_ERROR([
        Libuv headers not found.
        Please copy them (e.g. libuv/include) into your PHP build include path.
      ])
    fi

    dnl Check for libuv library
    AC_CHECK_LIB([uv], [uv_run], [have_uv=yes], [have_uv=no])
    if test "$have_uv" = "no"; then
      AC_MSG_ERROR([libuv library not found in the linker path.])
    fi

    dnl Define symbol to enable libuv code.
    AC_DEFINE([HAVE_ASYNC_LIBUV], 1, [Build True Async API with libuv])

    dnl Link against needed libraries.
    PHP_ADD_LIBRARY([uv], 1, ASYNC_SHARED_LIBADD)
    PHP_ADD_LIBRARY([Dbghelp], 1, ASYNC_SHARED_LIBADD)
    PHP_ADD_LIBRARY([Userenv], 1, ASYNC_SHARED_LIBADD)

    dnl Add libuv-specific reactor code.
    PHP_ADD_SOURCES([async/libuv], [libuv_reactor.c])
  fi
fi
