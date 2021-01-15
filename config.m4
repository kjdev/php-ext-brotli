dnl config.m4 for extension brotli

dnl Check PHP version ID:
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION_ID=`grep 'PHP_VERSION_ID' $phpincludedir/main/php_version.h | sed -e 's/.* \([[0-9]]*\)/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION_ID=`$PHP_CONFIG --vernum 2>/dev/null`
fi

if test x"$PHP_VERSION_ID" = "x"; then
    AC_MSG_WARN([none])
else
    AC_MSG_RESULT($PHP_VERSION_ID)
fi

if test $PHP_VERSION_ID -lt 50000; then
   AC_MSG_ERROR([need at least PHP 5 or newer])
fi

PHP_ARG_ENABLE(brotli, whether to enable brotli support,
[  --enable-brotli         Enable brotli support])

PHP_ARG_WITH(libbrotli, whether to use system brotli library,
[  --with-libbrotli        Use libbrotli], no, no)

if test "$PHP_BROTLI" != "no"; then

  BROTLI_MIN_VERSION=0.6

  if test "$PHP_LIBBROTLI" != "no"; then
    AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

    AC_MSG_CHECKING(for libbrotlienc)
    if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists libbrotlienc; then
      if $PKG_CONFIG libbrotlienc --atleast-version $BROTLI_MIN_VERSION; then
        LIBBROTLIENC_CFLAGS=`$PKG_CONFIG libbrotlienc --cflags`
        LIBBROTLIENC_LIBS=`$PKG_CONFIG libbrotlienc --libs`
        LIBBROTLIENC_VERSION=`$PKG_CONFIG libbrotlienc --modversion`
        AC_MSG_RESULT(from pkgconfig: version $LIBBROTLIENC_VERSION found)
      else
        AC_MSG_ERROR(system libbrotlienc must be upgraded to version >= $BROTLI_MIN_VERSION)
      fi
    else
      AC_MSG_ERROR(system libbrotlienc not found)
    fi
    PHP_EVAL_INCLINE($LIBBROTLIENC_CFLAGS)
    PHP_EVAL_LIBLINE($LIBBROTLIENC_LIBS, BROTLI_SHARED_LIBADD)

    AC_MSG_CHECKING(for libbrotlidec)
    if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists libbrotlidec; then
      if $PKG_CONFIG libbrotlidec --atleast-version $BROTLI_MIN_VERSION; then
        LIBBROTLIDEC_CFLAGS=`$PKG_CONFIG libbrotlidec --cflags`
        LIBBROTLIDEC_LIBS=`$PKG_CONFIG libbrotlidec --libs`
        LIBBROTLIDEC_VERSION=`$PKG_CONFIG libbrotlidec --modversion`
        AC_MSG_RESULT(from pkgconfig: version $LIBBROTLIDEC_VERSION found)
      else
        AC_MSG_ERROR(system libbrotlienc must be upgraded to version >= $BROTLI_MIN_VERSION)
      fi
    else
      AC_MSG_ERROR(system libbrotlienc not found)
    fi
    PHP_EVAL_INCLINE($LIBBROTLIDEC_CFLAGS)
    PHP_EVAL_LIBLINE($LIBBROTLIDEC_LIBS, BROTLI_SHARED_LIBADD)
    AC_DEFINE_UNQUOTED(BROTLI_LIB_VERSION, "$LIBBROTLIDEC_VERSION", [system library version])
  else
    AC_MSG_CHECKING(for brotli)
    AC_MSG_RESULT(use bundled copy)

    BROTLI_COMMON_SOURCES="brotli/c/common/constants.c brotli/c/common/context.c brotli/c/common/dictionary.c brotli/c/common/platform.c brotli/c/common/transform.c"
    BROTLI_ENC_SOURCES="brotli/c/enc/backward_references.c brotli/c/enc/backward_references_hq.c brotli/c/enc/bit_cost.c brotli/c/enc/block_splitter.c brotli/c/enc/brotli_bit_stream.c brotli/c/enc/cluster.c brotli/c/enc/command.c brotli/c/enc/compress_fragment.c brotli/c/enc/compress_fragment_two_pass.c brotli/c/enc/dictionary_hash.c brotli/c/enc/encode.c brotli/c/enc/encoder_dict.c brotli/c/enc/entropy_encode.c brotli/c/enc/fast_log.c brotli/c/enc/histogram.c brotli/c/enc/literal_cost.c brotli/c/enc/memory.c brotli/c/enc/metablock.c brotli/c/enc/static_dict.c brotli/c/enc/utf8_util.c"
    BROTLI_DEC_SOURCES="brotli/c/dec/bit_reader.c brotli/c/dec/decode.c brotli/c/dec/huffman.c brotli/c/dec/state.c"
  fi

  PHP_SUBST(BROTLI_SHARED_LIBADD)

  PHP_NEW_EXTENSION(brotli, brotli.c $BROTLI_COMMON_SOURCES $BROTLI_ENC_SOURCES $BROTLI_DEC_SOURCES, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)

  if test -n "$BROTLI_COMMON_SOURCES" ; then
    PHP_ADD_INCLUDE([$ext_srcdir/brotli/c/include])
  fi

  AC_MSG_CHECKING([for APCu includes])
  if test -f "$phpincludedir/ext/apcu/apc_serializer.h"; then
    apc_inc_path="$phpincludedir"
    AC_MSG_RESULT([APCu in $apc_inc_path])
    AC_DEFINE(HAVE_APCU_SUPPORT, 1, [Whether to enable APCu support])
  else
    AC_MSG_RESULT([not found])
  fi
fi
