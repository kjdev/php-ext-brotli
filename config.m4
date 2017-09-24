dnl config.m4 for extension brotli

dnl Check PHP version:
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9\.]]*\)".*/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
fi

if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([none])
else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
fi

if test $PHP_MAJOR_VERSION -lt 5; then
   AC_MSG_ERROR([need at least PHP 5 or newer])
fi

PHP_ARG_ENABLE(brotli, whether to enable brotli support,
[  --enable-brotli         Enable brotli support])

if test "$PHP_BROTLI" != "no"; then

    BROTLI_COMMON_SOURCES="brotli/c/common/dictionary.c"
    BROTLI_ENC_SOURCES="brotli/c/enc/backward_references.c brotli/c/enc/backward_references_hq.c brotli/c/enc/bit_cost.c brotli/c/enc/block_splitter.c brotli/c/enc/brotli_bit_stream.c brotli/c/enc/cluster.c brotli/c/enc/compress_fragment.c brotli/c/enc/compress_fragment_two_pass.c brotli/c/enc/dictionary_hash.c brotli/c/enc/encode.c brotli/c/enc/entropy_encode.c brotli/c/enc/histogram.c brotli/c/enc/literal_cost.c brotli/c/enc/memory.c brotli/c/enc/metablock.c brotli/c/enc/static_dict.c brotli/c/enc/utf8_util.c"
    BROTLI_DEC_SOURCES="brotli/c/dec/bit_reader.c brotli/c/dec/decode.c brotli/c/dec/huffman.c brotli/c/dec/state.c"

    PHP_SUBST(BROTLI_SHARED_LIBADD)

    PHP_NEW_EXTENSION(brotli, brotli.c $BROTLI_COMMON_SOURCES $BROTLI_ENC_SOURCES $BROTLI_DEC_SOURCES, $ext_shared)

    PHP_ADD_INCLUDE([$ext_srcdir/brotli/c/include])
fi
