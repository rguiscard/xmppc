AC_INIT([xmppc], [0.1.0], [stefan@debxwoody.de])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CC                                                                                     
AC_PROG_LIBTOOL
AC_SYS_LARGEFILE

AC_CANONICAL_HOST
PLATFORM="unknown"
AS_CASE([$host_os],
    [freebsd*], [PLATFORM="freebsd"],
    [openbsd*], [PLATFORM="openbsd"],
    [darwin*], [PLATFORM="osx"],
    [cygwin], [PLATFORM="cygwin"],
    [PLATFORM="nix"])

PACKAGE_STATUS="development"
#PACKAGE_STATUS="release"

AM_CFLAGS="-Wall -Wno-deprecated-declarations -pedantic -std=c11"
AS_IF([test "x$PACKAGE_STATUS" = xdevelopment],
    [AM_CFLAGS="$AM_CFLAGS -Wunused -Werror -g -O0"])

AS_IF([test "x$PACKAGE_STATUS" = xdevelopment],
    [AC_DEFINE([XMPPC_DEVELOPMENT], [1], [XMPPC Development])])

# libstrophe
PKG_CHECK_MODULES([libstrophe], [libstrophe >= 0.9.2],
    [LIBS="$libstrophe_LIBS $LIBS" CFLAGS="$CFLAGS $libstrophe_CFLAGS" AC_DEFINE([HAVE_LIBSTROPHE], [1], [libstrophe])],
    [AC_MSG_ERROR([libstrophe in version >= 0.9.2 not found, is required xmppc])])

# glib
PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.40],
    [LIBS="$glib_LIBS $LIBS" CFLAGS="$CFLAGS $glib_CFLAGS"],
    [AC_MSG_ERROR([glib 2.40 or higher is required for xmppc])])

AM_CONDITIONAL([BUILD_PGP], [false])
if test "x$enable_pgp" != xno; then
    AC_CHECK_LIB([gpgme], [main],
        [AM_CONDITIONAL([BUILD_PGP], [true])
         AC_DEFINE([HAVE_LIBGPGME], [1], [Have libgpgme])
         AC_PATH_PROG([GPGME_CONFIG], [gpgme-config], ["failed"])
         AS_IF([test "x$GPGME_CONFIG" = xfailed],
            [LIBS="-lgpgme $LIBS"],
            [LIBS="`$GPGME_CONFIG --libs` $LIBS" AM_CPPFLAGS="`$GPGME_CONFIG --cflags` $AM_CPPFLAGS"])],
        [AS_IF([test "x$enable_pgp" = xyes],
            [AC_MSG_ERROR([libgpgme is required for pgp support])],
            [AC_MSG_NOTICE([libgpgme not found, pgp support not enabled])])])
fi


AC_SUBST(AM_LDFLAGS)                                                                              
AC_SUBST(AM_CFLAGS)                                                                               
AC_SUBST(AM_CPPFLAGS)

AC_CHECK_FUNCS([strdup])

AC_OUTPUT

echo ""
echo "PLATFORM       : $host_os"
echo "PACKAGE_STATUS : $PACKAGE_STATUS"
echo "AM_CFLAGS      : $AM_CFLAGS"
echo "AM_CPPFLAGS    : $AM_CPPFLAGS"
echo "AM_LDFLAGS     : $AM_LDFLAGS"
echo "LIBS           : $LIBS"

