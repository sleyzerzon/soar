#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([SoarKernel], [8.6.0])
AM_INIT_AUTOMAKE([SoarKernel], [8.6.0])
AC_CONFIG_SRCDIR([src/soar.h])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_DISABLE_SHARED
AC_DEFINE([UNIX], [1], [BUGBUG: This needs to be eventually removed.])
AC_DEFINE([USE_TCL], [1], [BUGBUG: This needs to be eventually removed.])
AC_DEFINE([_DEBUG], [1], [BUGBUG: This needs to be eventually removed.])
AC_PROG_LIBTOOL
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stddef.h sys/time.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_VPRINTF
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_PATH_X
AC_STRUCT_TM
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([endpwent getcwd gethostname gettimeofday memset modf sqrt strchr strstr strtol])

AC_OUTPUT(Makefile src/Makefile)
