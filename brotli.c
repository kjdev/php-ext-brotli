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

/* brotli */
#include "brotli/enc/encode.h"
#include "brotli/dec/decode.h"
#include "brotli/tools/version.h"

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
    REGISTER_LONG_CONSTANT("BROTLI_GENERIC", BROTLI_MODE_GENERIC,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_TEXT", BROTLI_MODE_TEXT,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_FONT", BROTLI_MODE_FONT,
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
    php_info_print_table_row(2, "Library Version", BROTLI_VERSION);
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
    char *in;
#if ZEND_MODULE_API_NO >= 20141001
    size_t in_size;
#else
    int in_size;
#endif
    long quality = BROTLI_DEFAULT_QUALITY;
    long mode =  BROTLI_MODE_GENERIC;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s|ll", &in, &in_size,
                              &quality, &mode) == FAILURE) {
        RETURN_FALSE;
    }

    size_t out_size = BrotliEncoderMaxCompressedSize(in_size);
    char *out = (char *)emalloc(out_size);
    if (!out) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "Brotli compress memory allocate failed\n");
        RETURN_FALSE;
    }

    if (mode != BROTLI_MODE_GENERIC &&
        mode != BROTLI_MODE_TEXT &&
        mode != BROTLI_MODE_FONT) {
        mode = BROTLI_MODE_GENERIC;
    }

    if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) {
        quality = BROTLI_DEFAULT_QUALITY;
    }

    int lgwin = BROTLI_DEFAULT_WINDOW;

    if (!BrotliEncoderCompress((int)quality, lgwin, (BrotliEncoderMode)mode,
                               in_size, (const uint8_t*)in,
                               &out_size, (uint8_t*)out)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "Brotli compress failed\n");
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

static ZEND_FUNCTION(brotli_uncompress)
{
    long max_size = 0;
    char *in;
#if ZEND_MODULE_API_NO >= 20141001
    size_t in_size;
    smart_string out = {0};
#else
    int in_size;
    smart_str out = {0};
#endif

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
                              &in, &in_size, &max_size) == FAILURE) {
        RETURN_FALSE;
    }

    if (max_size && max_size < in_size) {
        in_size = max_size;
    }

    BrotliState *state = BrotliCreateState(NULL, NULL, NULL);
    if (!state) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "Invalid Brotli state\n");
        RETURN_FALSE;
    }

    const size_t kFileBufferSize = 65536;

    size_t available_in = in_size;
    const uint8_t *next_in = (const uint8_t *)in;
    size_t buffer_size = kFileBufferSize;
    uint8_t *buffer = (uint8_t *)emalloc(buffer_size);

    BrotliResult result = BROTLI_RESULT_NEEDS_MORE_OUTPUT;
    while (result == BROTLI_RESULT_NEEDS_MORE_OUTPUT) {
        size_t available_out = buffer_size;
        uint8_t *next_out = buffer;
        size_t total_out = 0;
        result = BrotliDecompressStream(&available_in, &next_in,
                                        &available_out, &next_out,
                                        &total_out, state);
        size_t used_out = buffer_size - available_out;
        if (used_out != 0) {
#if ZEND_MODULE_API_NO >= 20141001
            smart_string_appendl(&out, buffer, used_out);
#else
            smart_str_appendl(&out, buffer, used_out);
#endif
        }
    }

    BrotliDestroyState(state);
    efree(buffer);

    if (result != BROTLI_RESULT_SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "Brotli decompress failed\n");
#if ZEND_MODULE_API_NO >= 20141001
        smart_string_free(&out);
#else
        smart_str_free(&out);
#endif
        RETURN_FALSE;
    }

#if ZEND_MODULE_API_NO >= 20141001
    RETVAL_STRINGL(out.c, out.len);
#else
    RETVAL_STRINGL(out.c, out.len, 1);
#endif

#if ZEND_MODULE_API_NO >= 20141001
    smart_string_free(&out);
#else
    smart_str_free(&out);
#endif
}
