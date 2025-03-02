dnl vim: ft=config

PHP_ARG_ENABLE(async, whether to enable True Async API,
[  --enable-async          Enable True Async API], no, no)

PHP_ARG_WITH(async_libuv, whether to build with libuv support,
[  --with-async-libuv[=DIR]      Build True Async API with libuv support], no, no)

if test "$PHP_ASYNC" != "no"; then
  dnl Add main source files
  PHP_ADD_SOURCES("async", async.c reactor.c scheduler.c, ASYNC_CFLAGS)
  PHP_ADD_SOURCES("async/internal", allocator.c circular_buffer.c, ASYNC_CFLAGS)
  PHP_ADD_SOURCES("async/php_layer", \
    closure.c notifier.c resume.c reactor_handles.c exceptions.c module_entry.c \
    zend_common.c walker.c future.c pool.c context.c channel.c, ASYNC_CFLAGS)

  dnl Define the main macro
  AC_DEFINE(PHP_ASYNC, 1, [Enable True Async API])

  dnl Install header files
  PHP_INSTALL_HEADERS([ext/async], php_async.h php_reactor.h php_scheduler.h)
  PHP_INSTALL_HEADERS([ext/async/libuv], libuv_reactor.h)
  PHP_INSTALL_HEADERS([ext/async/php_layer], \
    closure.h channel.h reactor_handles.h exceptions.h module_entry.h \
    notifier.h resume.h walker.h future.h pool.h context.h)

  dnl Register the extension
  PHP_NEW_EXTENSION(async, $ext_src, $ext_shared)
fi

if test "$PHP_ASYNC_LIBUV" != "no"; then
  dnl Check async dependency
  if test "$PHP_ASYNC" = "no"; then
    AC_MSG_ERROR([libuv requires True Async API to be enabled. Use --enable-async])
  fi

  dnl Search for libuv using pkg-config
  PKG_CHECK_MODULES([LIBUV], [libuv >= 1.0.0], [
    PHP_EVAL_INCLINE($LIBUV_CFLAGS)
    PHP_EVAL_LIBLINE($LIBUV_LIBS, ASYNC_LIBUV_SHARED_LIBADD)
    UV_FOUND=yes
  ], [
    UV_FOUND=no
  ])

  dnl Fallback: Manual search for libuv
  if test "$UV_FOUND" = "no"; then
    if test -r "$PHP_ASYNC_LIBUV/include/uv.h"; then
      UV_INC_DIR="$PHP_ASYNC_LIBUV/include"
      UV_LIB_DIR="$PHP_ASYNC_LIBUV/$PHP_LIBDIR"
    else
      UV_SEARCH_DIRS="/usr/local /usr"
      for dir in $UV_SEARCH_DIRS; do
        if test -r "$dir/include/uv.h"; then
          UV_INC_DIR="$dir/include"
          UV_LIB_DIR="$dir/$PHP_LIBDIR"
          UV_FOUND=yes
          break
        fi
      done
    fi

    if test "$UV_FOUND" = "no"; then
      AC_MSG_ERROR([uv.h not found. Please install libuv development files])
    fi

    PHP_ADD_INCLUDE($UV_INC_DIR)
    PHP_ADD_LIBRARY_WITH_PATH(uv, $UV_LIB_DIR, ASYNC_LIBUV_SHARED_LIBADD)
  fi

  dnl Check for required libuv functions
  CFLAGS_SAVE=$CFLAGS
  LIBS_SAVE=$LIBS
  CFLAGS="$CFLAGS $LIBUV_CFLAGS"
  LIBS="$LIBS $LIBUV_LIBS"

  AC_CHECK_LIB(uv, uv_loop_init, [
    AC_DEFINE(PHP_ASYNC_LIBUV, 1, [Enable libuv support])
  ], [
    AC_MSG_ERROR([Invalid libuv library version])
  ])

  AC_CHECK_LIB(uv, uv_async_send, [], [
    AC_MSG_ERROR([libuv missing required uv_async_send function])
  ])

  dnl Restore original flags
  CFLAGS=$CFLAGS_SAVE
  LIBS=$LIBS_SAVE

  dnl Add libuv-specific sources
  PHP_ADD_SOURCES("async/libuv", libuv_reactor.c, ASYNC_LIBUV_CFLAGS)
fi