#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#if ZEND_MODULE_API_NO >= 20141001
#include <ext/standard/php_smart_string.h>
#else
#include <ext/standard/php_smart_str.h>
#endif
#include "php_brotli.h"

#ifdef __cplusplus
}
#endif

/* brotli */
#include "brotli/enc/encode.h"
#include "brotli/dec/decode.h"

static ZEND_FUNCTION(brotli_compress);
static ZEND_FUNCTION(brotli_uncompress);

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_compress, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, quality)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_uncompress, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

static zend_function_entry brotli_functions[] = {
    ZEND_FE(brotli_compress, arginfo_brotli_compress)
    ZEND_FE(brotli_uncompress, arginfo_brotli_uncompress)
    ZEND_FE_END
};

ZEND_MINIT_FUNCTION(brotli)
{
    REGISTER_LONG_CONSTANT("BROTLI_GENERIC",
                           brotli::BrotliParams::Mode::MODE_GENERIC,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_TEXT",
                           brotli::BrotliParams::Mode::MODE_TEXT,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_FONT",
                           brotli::BrotliParams::Mode::MODE_FONT,
                           CONST_CS | CONST_PERSISTENT);
    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(brotli)
{
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(brotli)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Brotli support", "enabled");
    php_info_print_table_row(2, "Extension Version", BROTLI_EXT_VERSION);
    php_info_print_table_row(2, "Library Version", BROTLI_LIB_VERSION);
    php_info_print_table_end();
}

zend_module_entry brotli_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "brotli",
    brotli_functions,
    ZEND_MINIT(brotli),
    ZEND_MSHUTDOWN(brotli),
    NULL,
    NULL,
    ZEND_MINFO(brotli),
#if ZEND_MODULE_API_NO >= 20010901
    BROTLI_EXT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BROTLI
ZEND_GET_MODULE(brotli)
#endif

static ZEND_FUNCTION(brotli_compress)
{
    long quality = 11;
    long mode = -1;
    char *in, *out = NULL;
    int in_size;
    size_t out_size = 0;
    brotli::BrotliParams params;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s|ll", &in, &in_size,
                              &quality, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    if (quality > 0) {
        params.quality = quality;
    }

    if (mode != -1 &&
        (mode == brotli::BrotliParams::Mode::MODE_GENERIC ||
         mode == brotli::BrotliParams::Mode::MODE_TEXT ||
         mode == brotli::BrotliParams::Mode::MODE_FONT)) {
        params.mode = mode;
    }

    out_size = 1.2 * in_size + 10240;
    out = (char *)emalloc(out_size);

    if (!BrotliCompressBuffer(params, in_size, (const uint8_t*)in,
                              &out_size, (uint8_t*)out)) {
        php_printf("ERR: compressBuffer\n");
        efree(out);
        RETURN_FALSE;
    }

#if ZEND_MODULE_API_NO >= 20141001
    RETVAL_STRINGL(out, out_size);
#else
    RETVAL_STRINGL(out, out_size, 1);
#endif
    efree(out);
}

static int
php_brotli_output_callback(void *data, const uint8_t *buf, size_t count)
{
#if ZEND_MODULE_API_NO >= 20141001
    smart_string *output = (smart_string *)data;
    smart_string_appendl(output, buf, count);
#else
    smart_str *output = (smart_str *)data;
    smart_str_appendl(output, buf, count);
#endif
    return (int)count;
}

static ZEND_FUNCTION(brotli_uncompress)
{
    long max_size = 0;
    char *in;
    int in_size;
#if ZEND_MODULE_API_NO >= 20141001
    smart_string out = {0};
#else
    smart_str out = {0};
#endif
    BrotliMemInput mem_in;
    BrotliInput input;
    BrotliOutput output;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
                              &in, &in_size, &max_size) == FAILURE) {
        RETURN_FALSE;
    }

    if (max_size && max_size < in_size) {
        in_size = max_size;
    }

    input = BrotliInitMemInput((const uint8_t*)in, in_size, &mem_in);

    output.cb_ = &php_brotli_output_callback;
    output.data_ = &out;

    if (!BrotliDecompress(input, output)) {
#if ZEND_MODULE_API_NO >= 20141001
        smart_string_free(&out);
#else
        smart_str_free(&out);
#endif
        RETURN_FALSE;
    }

#if ZEND_MODULE_API_NO >= 20141001
    RETURN_STRINGL(out.c, out.len);
#else
    RETURN_STRINGL(out.c, out.len, 1);
#endif

#if ZEND_MODULE_API_NO >= 20141001
    smart_string_free(&out);
#else
    smart_str_free(&out);
#endif
}
