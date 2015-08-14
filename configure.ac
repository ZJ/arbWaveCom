#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([]arbWaveCom, [1.1], [http://github.com/ZJ/arbWaveCom/issues], [awgcom])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign color-tests])
AC_CONFIG_SRCDIR([src/driver.c])
AC_CONFIG_HEADERS([config.h])

# Check what host we're building
AM_CONDITIONAL([MINGW_HOST], [test host-$host = host-i686-w64-mingw32 -o host-$host = host-x86_64-w64-mingw32])
AM_COND_IF([MINGW_HOST], [AC_MSG_NOTICE([You are cross-compiling on mingw--unit tests will be unavailable])])
AM_COND_IF([MINGW_HOST], [AC_DEFINE([ON_MINGW_HOST], [1], [Detected cross on mingw]) AC_SUBST(mingwldflags,[-static])])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR
LT_INIT()

# Check for Check for Unit Testing
PKG_CHECK_MODULES([CHECK], [check >= 0.9.0],[have_check="yes"],
  AC_MSG_WARN([Check not found; cannot run unit tests!])
  [have_check="no"]
)
AM_CONDITIONAL(HAVE_CHECK, test x"$have_check" = "xyes")

# Checks for libraries.
AC_CHECK_LIB([m], [exp])

# Checks for header files.
AC_CHECK_HEADER([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 src/defOptions/Makefile
 src/genBinary/Makefile
 tests/Makefile
])
AC_OUTPUT
