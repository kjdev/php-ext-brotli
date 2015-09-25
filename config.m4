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

    PHP_REQUIRE_CXX()

    BROTLI_CXXFLAGS="-std=c++11 -fpermissive"
    BROTLI_SOURCES="brotli/enc/backward_references.cc brotli/enc/block_splitter.cc brotli/enc/brotli_bit_stream.cc brotli/enc/encode.cc brotli/enc/encode_parallel.cc brotli/enc/entropy_encode.cc brotli/enc/histogram.cc brotli/enc/literal_cost.cc brotli/enc/metablock.cc brotli/enc/static_dict.cc brotli/enc/streams.cc brotli/dec/bit_reader.c brotli/dec/decode.c brotli/dec/huffman.c brotli/dec/state.c brotli/dec/streams.c"

    PHP_ADD_LIBRARY(stdc++, , BROTLI_SHARED_LIBADD)
    PHP_SUBST(BROTLI_SHARED_LIBADD)

    PHP_NEW_EXTENSION(brotli, brotli.cc $BROTLI_SOURCES, $ext_shared, ,$BROTLI_CXXFLAGS)

fi
