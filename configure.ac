AC_REVISION($Id$)
AC_INIT(sick-driver, 0.0, roland DOT philippsen AT gmx DOT net)
AC_PREREQ(2.57)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

AC_LANG(C++)
AC_PROG_CC
AC_PROG_CXX
case $host_os in
  darwin*) AC_DISABLE_SHARED;;
esac
AC_PROG_LIBTOOL
AC_PROG_INSTALL

CFLAGS="$CFLAGS -pipe"

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug], [GCC options -g -O0 (else -O3)]),
  [ CPPFLAGS="$CPPFLAGS -DCRB_DEBUG"
    CFLAGS="$CFLAGS -g -O0" ],
  [ CFLAGS="$CFLAGS -O3" ])

AC_ARG_ENABLE(pedantic,
  AC_HELP_STRING([--enable-pedantic], [GCC option -pedantic (else -Wall)]),
  [ CFLAGS="$CFLAGS -pedantic" ],
  [ CFLAGS="$CFLAGS -Wall" ])

case "$host_os" in
  linux*)  AC_MSG_NOTICE([detected Linux])
           CPPFLAGS="$CPPFLAGS -DLINUX"
	   LDFLAGS="-lpthread $LDFLAGS"
	    ;;
  darwin*) AC_MSG_NOTICE([detected OSX])
           CPPFLAGS="$CPPFLAGS -DOSX"
	    ;;
  *)       AC_MSG_ERROR([unknown host_os $host_os, update configure.ac]);;
esac

CXXFLAGS="$CFLAGS"
AC_SUBST(PACKAGE_VERSION)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES(Makefile)
AC_OUTPUT
