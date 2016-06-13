PHP_ARG_WITH(gmpi, for GNU MP support,
[  --with-gmpi[=DIR]          Include GNU MP support])

if test "$PHP_GMPI" != "no"; then

  for i in $PHP_GMPI /usr/local /usr; do
    test -f $i/include/gmp.h && GMPI_DIR=$i && break
  done

  if test -z "$GMPI_DIR"; then
    AC_MSG_ERROR(Unable to locate gmp.h)
  fi

  PHP_CHECK_LIBRARY(gmp, __gmpz_rootrem,
  [],[
    AC_MSG_ERROR([GNU MP Library version 4.2 or greater required.])
  ],[
    -L$GMPI_DIR/$PHP_LIBDIR
  ])

  PHP_ADD_LIBRARY_WITH_PATH(gmp, $GMPI_DIR/$PHP_LIBDIR, GMPI_SHARED_LIBADD)
  PHP_ADD_INCLUDE($GMPI_DIR/include)

  PHP_NEW_EXTENSION(gmpi, gmpi.c gmpi-int.c gmpi-float.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  PHP_SUBST(GMPI_SHARED_LIBADD)
  AC_DEFINE(HAVE_GMPI, 1, [ ])
fi
