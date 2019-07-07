PHP_ARG_ENABLE(piof, whether to enable my extension,
[ --enable-piof  Enable piof])

if test "$PHP_PIOF" = "yes"; then
  AC_DEFINE(HAVE_PIOF, 1, [Whether you have piof])
  PHP_NEW_EXTENSION(piof, piof.c cJSON.c base64.c piof_globals.c, $ext_shared)
  CFLAGS="$CFLAGS -ldl"
fi
