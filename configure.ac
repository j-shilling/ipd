#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ipd], [0.1], [shilling.jake@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([CURL], [libcurl])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h stdio.h string.h errno.h regex.h unistd.h time.h sys/types.h ctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([regcomp])
AC_CHECK_FUNCS([asprintf])

AC_OUTPUT([Makefile
	   src/Makefile])
