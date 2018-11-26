#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include <php.h>
#include <SAPI.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#if ZEND_MODULE_API_NO >= 20141001
#include <ext/standard/php_smart_string.h>
#else
#include <ext/standard/php_smart_str.h>
#endif
#include "php_brotli.h"

/* brotli */
#include "brotli/encode.h"
#include "brotli/decode.h"

ZEND_DECLARE_MODULE_GLOBALS(brotli);

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

#if PHP_VERSION_ID > 50400 // Output Handler: 5.4+

#define PHP_BROTLI_OUTPUT_HANDLER "ob_brotli_handler"

typedef struct _php_brotli_context {
    BrotliEncoderState *state;
    size_t available_in;
    const uint8_t *next_in;
    size_t available_out;
    uint8_t *next_out;
    uint8_t *output;
} php_brotli_context;

static int php_brotli_output_encoding(void)
{
#if PHP_MAJOR_VERSION >= 7
    zval *enc;
#else
    zval **enc;
    TSRMLS_FETCH();
#endif

    if (!BROTLI_G(compression_coding)) {
#if PHP_MAJOR_VERSION >= 7
        if ((Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY
             || zend_is_auto_global_str(ZEND_STRL("_SERVER")))
            && (enc = zend_hash_str_find(
                    Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]),
                    "HTTP_ACCEPT_ENCODING",
                    sizeof("HTTP_ACCEPT_ENCODING") - 1))) {
            convert_to_string(enc);
            if (strstr(Z_STRVAL_P(enc), "br")) {
                BROTLI_G(compression_coding) = 1;
            }
        }
#else
        if ((PG(http_globals)[TRACK_VARS_SERVER]
             || zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC)) &&
            SUCCESS == zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_ACCEPT_ENCODING", sizeof("HTTP_ACCEPT_ENCODING"), (void *) &enc)) {
            convert_to_string(*enc);
            if (strstr(Z_STRVAL_PP(enc), "br")) {
                BROTLI_G(compression_coding) = 1;
            }
        }
#endif
    }

    return BROTLI_G(compression_coding);
}

static int php_brotli_encoder_create(php_brotli_context *ctx)
{
#if PHP_MAJOR_VERSION < 7
    TSRMLS_FETCH();
#endif

    ctx->state = BrotliEncoderCreateInstance(NULL, NULL, NULL);
    if (!ctx->state) {
        return FAILURE;
    }

    long quality = BROTLI_G(output_compression_level);
    if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) {
        quality = BROTLI_DEFAULT_QUALITY;
    }
    int lgwin = BROTLI_DEFAULT_WINDOW;

    BrotliEncoderSetParameter(ctx->state, BROTLI_PARAM_QUALITY, quality);
    BrotliEncoderSetParameter(ctx->state, BROTLI_PARAM_LGWIN, lgwin);

    return SUCCESS;
}

static void php_brotli_encoder_destroy(php_brotli_context *ctx)
{
    if (ctx->state) {
        BrotliEncoderDestroyInstance(ctx->state);
        ctx->state = NULL;
    }
}

static int php_brotli_output_handler(void **handler_context,
                                     php_output_context *output_context)
{
    php_brotli_context *ctx = *(php_brotli_context **)handler_context;
#if PHP_MAJOR_VERSION < 7
    TSRMLS_FETCH();
#endif

    if (!php_brotli_output_encoding()) {
        if ((output_context->op & PHP_OUTPUT_HANDLER_START)
            &&  (output_context->op
                 != (PHP_OUTPUT_HANDLER_START
                     |PHP_OUTPUT_HANDLER_CLEAN|PHP_OUTPUT_HANDLER_FINAL))) {
            sapi_add_header_ex(ZEND_STRL("Vary: Accept-Encoding"),
                               1, 0 TSRMLS_CC);
        }
        return FAILURE;
    }

    if (!BROTLI_G(output_compression)) {
        return FAILURE;
    }

    if (output_context->op & PHP_OUTPUT_HANDLER_START) {
        if (php_brotli_encoder_create(ctx) != SUCCESS) {
            return FAILURE;
        }
    }

    if (!(output_context->op & PHP_OUTPUT_HANDLER_CLEAN)) {
        if (output_context->in.used) {
            size_t size
                = BrotliEncoderMaxCompressedSize(output_context->in.used);
            if (!ctx->output) {
                ctx->output = (uint8_t *)emalloc(size);
                ctx->available_out = size;
                ctx->next_out = ctx->output;
            } else {
                ctx->available_out += size;
                ctx->output = (uint8_t *)erealloc(ctx->output,
                                                  ctx->available_out);
                if (!ctx->output) {
                    php_brotli_encoder_destroy(ctx);
                    return FAILURE;
                }
                ctx->next_out = ctx->output;
            }

            // append input
            ctx->available_in = output_context->in.used;
            ctx->next_in = (uint8_t *)output_context->in.data;
        } else {
            ctx->available_in = 0;
            ctx->next_in = NULL;
        }

        if (!BrotliEncoderCompressStream(ctx->state,
                                         (output_context->op
                                          & PHP_OUTPUT_HANDLER_FINAL)
                                         ? BROTLI_OPERATION_FINISH
                                         : BROTLI_OPERATION_PROCESS,
                                         &ctx->available_in,
                                         &ctx->next_in,
                                         &ctx->available_out,
                                         &ctx->next_out,
                                         NULL)) {
            php_brotli_encoder_destroy(ctx);
            return FAILURE;
        }

        if (output_context->op & PHP_OUTPUT_HANDLER_FINAL) {
            size_t size = (size_t)(ctx->next_out - ctx->output);
            output_context->out.data = ctx->output;
            output_context->out.used = size;
            output_context->out.free = 1;

            php_brotli_encoder_destroy(ctx);

            if (!SG(headers_sent)) {
                sapi_add_header_ex(ZEND_STRL("Content-Encoding: br"),
                                   1, 1 TSRMLS_CC);
                sapi_add_header_ex(ZEND_STRL("Vary: Accept-Encoding"),
                                   1, 0 TSRMLS_CC);
            }

            // reset
            BROTLI_G(output_compression) = 0;
        }
    } else {
        php_brotli_encoder_destroy(ctx);

        if (output_context->op & PHP_OUTPUT_HANDLER_FINAL) {
            // discard
            return SUCCESS;
        } else {
            // restart
            if (php_brotli_encoder_create(ctx) != SUCCESS) {
                return FAILURE;
            }
        }
    }

    return SUCCESS;
}

static php_brotli_context *php_brotli_output_handler_context_init(void)
{
    php_brotli_context *ctx
        = (php_brotli_context *)ecalloc(1, sizeof(php_brotli_context));
    ctx->state = NULL;
    ctx->available_in = 0;
    ctx->next_in = NULL;
    ctx->available_out = 0;
    ctx->next_out = NULL;
    ctx->output = NULL;
    return ctx;
}

static void php_brotli_output_handler_context_dtor(void *opaq TSRMLS_DC)
{
    php_brotli_context *ctx = (php_brotli_context *)opaq;

    if (ctx) {
        php_brotli_encoder_destroy(ctx);
        if (ctx->output) {
            efree(ctx->output);
        }
        efree(ctx);
    }
}

static php_output_handler*
php_brotli_output_handler_init(const char *handler_name,
                               size_t handler_name_len,
                               size_t chunk_size, int flags)
{
    php_output_handler *handler = NULL;
#if PHP_MAJOR_VERSION < 7
    TSRMLS_FETCH();
#endif

    BROTLI_G(handler_registered) = 1;

    handler = php_output_handler_create_internal(handler_name, handler_name_len,
                                                 php_brotli_output_handler,
                                                 chunk_size, flags TSRMLS_CC);
    if (!handler) {
        return NULL;
    }

    php_output_handler_set_context(handler,
                                   php_brotli_output_handler_context_init(),
                                   php_brotli_output_handler_context_dtor
                                   TSRMLS_CC);

    BROTLI_G(output_compression) = 1;

    return handler;
}

static void php_brotli_output_compression_start(void)
{
    zval zoh;
    php_output_handler *h;
#if PHP_MAJOR_VERSION < 7
    TSRMLS_FETCH();
#endif

    switch (BROTLI_G(output_compression)) {
        case 0:
            break;
        case 1:
            /* break omitted intentionally */
        default:
            if (php_brotli_output_encoding() &&
                (h = php_brotli_output_handler_init(
                    ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER),
                    PHP_OUTPUT_HANDLER_DEFAULT_SIZE,
                    PHP_OUTPUT_HANDLER_STDFLAGS))) {
                php_output_handler_start(h TSRMLS_CC);
            }
            break;
    }
}

static PHP_INI_MH(OnUpdate_brotli_output_compression)
{
    int int_value, status;
    char *ini_value;
    zend_long *p;
#ifndef ZTS
    char *base = (char *)mh_arg2;
#else
    char *base;
    base = (char *)ts_resource(*((int *) mh_arg2));
#endif

    if (new_value == NULL) {
        return FAILURE;
    }

#if PHP_MAJOR_VERSION >= 7
    if (!strncasecmp(ZSTR_VAL(new_value), "off", sizeof("off"))) {
        int_value = 0;
    } else if (!strncasecmp(ZSTR_VAL(new_value), "on", sizeof("on"))) {
        int_value = 1;
    } else if (zend_atoi(ZSTR_VAL(new_value), ZSTR_LEN(new_value))) {
        int_value = 1;
    } else {
        int_value = 0;
    }
#else
    if (!strncasecmp(new_value, "off", sizeof("off"))) {
        int_value = 0;
    } else if (!strncasecmp(new_value, "on", sizeof("on"))) {
        int_value = 1;
    } else if (zend_atoi(new_value, new_value_length)) {
        int_value = 1;
    } else {
        int_value = 0;
    }
#endif

    if (stage == PHP_INI_STAGE_RUNTIME) {
        status = php_output_get_status(TSRMLS_C);
        if (status & PHP_OUTPUT_SENT) {
            php_error_docref("ref.outcontrol" TSRMLS_CC, E_WARNING,
                             "Cannot change brotli.output_compression"
                             " - headers already sent");
            return FAILURE;
        }
    }

    p = (zend_long *)(base+(size_t) mh_arg1);
    *p = int_value;

    if (stage == PHP_INI_STAGE_RUNTIME && int_value) {
        if (!php_output_handler_started(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER)
                                        TSRMLS_CC)) {
            php_brotli_output_compression_start();
        }
    }

    return SUCCESS;
}

static int php_brotli_output_conflict(const char *handler_name, size_t handler_name_len TSRMLS_DC)
{
    if (php_output_get_level(TSRMLS_C)) {
        if (php_output_handler_conflict(handler_name, handler_name_len,
                                        ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER)
                                        TSRMLS_CC)
            || php_output_handler_conflict(handler_name, handler_name_len,
                                           ZEND_STRL("ob_gzhandler")
                                           TSRMLS_CC)) {
            return FAILURE;
        }
    }
    return SUCCESS;
}

PHP_INI_BEGIN()
  STD_PHP_INI_ENTRY("brotli.output_compression", "0",
                    PHP_INI_ALL, OnUpdate_brotli_output_compression,
                    output_compression, zend_brotli_globals, brotli_globals)
  STD_PHP_INI_ENTRY("brotli.output_compression_level", "-1",
                    PHP_INI_ALL, OnUpdateLong, output_compression_level,
                    zend_brotli_globals, brotli_globals)
PHP_INI_END()

static void php_brotli_init_globals(zend_brotli_globals *brotli_globals)
{
    brotli_globals->handler_registered = 0;
    brotli_globals->compression_coding = 0;
}
#endif

ZEND_MINIT_FUNCTION(brotli)
{
#if PHP_VERSION_ID > 50400
    ZEND_INIT_MODULE_GLOBALS(brotli, php_brotli_init_globals, NULL);
#endif

    REGISTER_LONG_CONSTANT("BROTLI_GENERIC", BROTLI_MODE_GENERIC,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_TEXT", BROTLI_MODE_TEXT,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_FONT", BROTLI_MODE_FONT,
                           CONST_CS | CONST_PERSISTENT);

#if PHP_VERSION_ID > 50400
    php_output_handler_alias_register(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER),
                                      php_brotli_output_handler_init TSRMLS_CC);
    php_output_handler_conflict_register(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER),
                                         php_brotli_output_conflict TSRMLS_CC);

    REGISTER_INI_ENTRIES();
#endif
    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(brotli)
{
#if PHP_VERSION_ID > 50400
    UNREGISTER_INI_ENTRIES();
#endif
    return SUCCESS;
}

ZEND_RINIT_FUNCTION(brotli)
{
#if PHP_VERSION_ID > 50400
    BROTLI_G(compression_coding) = 0;
    if (!BROTLI_G(handler_registered)) {
        php_brotli_output_compression_start();
    }
#endif
    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(brotli)
{
#if PHP_VERSION_ID > 50400
    BROTLI_G(handler_registered) = 0;
#endif
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
    ZEND_RINIT(brotli),
    ZEND_RSHUTDOWN(brotli),
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

    BrotliDecoderState *state = BrotliDecoderCreateInstance(NULL, NULL, NULL);
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

    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
    while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
      size_t available_out = buffer_size;
        uint8_t *next_out = buffer;
        size_t total_out = 0;
        result = BrotliDecoderDecompressStream(state, &available_in, &next_in,
                                               &available_out, &next_out,
                                               &total_out);
        size_t used_out = buffer_size - available_out;
        if (used_out != 0) {
#if ZEND_MODULE_API_NO >= 20141001
            smart_string_appendl(&out, buffer, used_out);
#else
            smart_str_appendl(&out, buffer, used_out);
#endif
        }
    }

    BrotliDecoderDestroyInstance(state);
    efree(buffer);

    if (result != BROTLI_DECODER_RESULT_SUCCESS) {
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
