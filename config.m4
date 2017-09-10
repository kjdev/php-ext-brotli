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

    BROTLI_COMMON_SOURCES="brotli/common/dictionary.c"
    BROTLI_ENC_SOURCES="brotli/enc/backward_references.c brotli/enc/backward_references_hq.c brotli/enc/bit_cost.c brotli/enc/block_splitter.c brotli/enc/brotli_bit_stream.c brotli/enc/cluster.c brotli/enc/compress_fragment.c brotli/enc/compress_fragment_two_pass.c brotli/enc/dictionary_hash.c brotli/enc/encode.c brotli/enc/entropy_encode.c brotli/enc/histogram.c brotli/enc/literal_cost.c brotli/enc/memory.c brotli/enc/metablock.c brotli/enc/static_dict.c brotli/enc/utf8_util.c"
    BROTLI_DEC_SOURCES="brotli/dec/bit_reader.c brotli/dec/decode.c brotli/dec/huffman.c brotli/dec/state.c"

    PHP_SUBST(BROTLI_SHARED_LIBADD)

    PHP_NEW_EXTENSION(brotli, brotli.c $BROTLI_COMMON_SOURCES $BROTLI_ENC_SOURCES $BROTLI_DEC_SOURCES, $ext_shared)

    PHP_ADD_INCLUDE([$ext_srcdir/brotli/include])
fi
