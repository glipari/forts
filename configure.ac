AC_INIT([EFFORTS], 1.0)
AC_CONFIG_SRCDIR([src/model.cpp])
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([TIPA], [tipa-1.0 >= 0.1])

AC_CHECK_PROGS(RANLIB, ranlib)
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT
