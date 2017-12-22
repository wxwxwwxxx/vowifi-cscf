#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([cscf], [0.1], [weixin 542841993@qq.com])
AC_CONFIG_SRCDIR([CSCF/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(cscf, 0.1)

CFLAGS="-g -Wall -Wformat -D_REENTRANT -Iinclude"
CXXFLAGS=$CFLAGS
CFLAGS="$CFLAGS -std=c99"

LIBS=


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [fmod])
AC_CHECK_LIB([gomp], [clock_gettime])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([bzero gettimeofday memchr memmove memset select strcasecmp strchr strncasecmp])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
