#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include <php.h>
#include <SAPI.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ext/standard/php_smart_string.h>
#if defined(HAVE_APCU_SUPPORT)
#include <ext/standard/php_var.h>
#include <ext/apcu/apc_serializer.h>
#include <zend_smart_str.h>
#endif
#include <Zend/zend_interfaces.h>
#if defined(USE_BROTLI_BUNDLED)
# pragma GCC visibility push(hidden)
#endif
#include "php_brotli.h"
#if defined(USE_BROTLI_BUNDLED)
# pragma GCC visibility pop
#endif

# pragma GCC diagnostic ignored "-Wpointer-sign"

ZEND_DECLARE_MODULE_GLOBALS(brotli);

static ZEND_FUNCTION(brotli_compress);
static ZEND_FUNCTION(brotli_uncompress);
static ZEND_FUNCTION(brotli_compress_init);
static ZEND_FUNCTION(brotli_compress_add);
static ZEND_FUNCTION(brotli_uncompress_init);
static ZEND_FUNCTION(brotli_uncompress_add);

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_brotli_compress, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 0, "BROTLI_COMPRESS_LEVEL_DEFAULT")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "BROTLI_GENERIC")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dict, IS_STRING, 1, "null")
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_compress, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, dict)
#endif
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_brotli_uncompress, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dict, IS_STRING, 1, "null")
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_uncompress, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, length)
#if defined(USE_BROTLI_DICTIONARY)
    ZEND_ARG_INFO(0, dict)
#endif
#endif
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_brotli_compress_init, 0, 0, Brotli\\Compress\\Context, MAY_BE_FALSE)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 0, "BROTLI_COMPRESS_LEVEL_DEFAULT")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "BROTLI_GENERIC")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dict, IS_STRING, 1, "null")
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_compress_init, 0, 0, 0)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, mode)
#if defined(USE_BROTLI_DICTIONARY)
    ZEND_ARG_INFO(0, dict)
#endif
#endif
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_brotli_compress_add, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
    ZEND_ARG_OBJ_INFO(0, context, Brotli\\Compress\\Context, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "BROTLI_FLUSH")
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_compress_add, 0, 0, 2)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, mode)
#endif
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_brotli_uncompress_init, 0, 0, Brotli\\UnCompress\\Context, MAY_BE_FALSE)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dict, IS_STRING, 1, "null")
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_uncompress_init, 0, 0, 0)
    ZEND_ARG_INFO(0, dict)
#endif
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_brotli_uncompress_add, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
    ZEND_ARG_OBJ_INFO(0, context, Brotli\\UnCompress\\Context, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "BROTLI_FLUSH")
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_uncompress_add, 0, 0, 2)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, mode)
#endif
ZEND_END_ARG_INFO()

#ifndef Z_PARAM_STR_OR_NULL
#define Z_PARAM_STR_OR_NULL(dest) Z_PARAM_STR_EX(dest, 1, 0)
#endif

#if defined(HAVE_APCU_SUPPORT)
static int APC_SERIALIZER_NAME(brotli)(APC_SERIALIZER_ARGS);
static int APC_UNSERIALIZER_NAME(brotli)(APC_UNSERIALIZER_ARGS);
#endif

static zend_function_entry brotli_functions[] = {
    ZEND_FE(brotli_compress, arginfo_brotli_compress)
    ZEND_FE(brotli_uncompress, arginfo_brotli_uncompress)
    ZEND_NS_FALIAS(BROTLI_NS, compress,
                   brotli_compress, arginfo_brotli_compress)
    ZEND_NS_FALIAS(BROTLI_NS, uncompress,
                   brotli_uncompress, arginfo_brotli_uncompress)
    ZEND_FE(brotli_compress_init, arginfo_brotli_compress_init)
    ZEND_FE(brotli_compress_add, arginfo_brotli_compress_add)
    ZEND_FE(brotli_uncompress_init, arginfo_brotli_uncompress_init)
    ZEND_FE(brotli_uncompress_add, arginfo_brotli_uncompress_add)
    ZEND_NS_FALIAS(BROTLI_NS, compress_init,
                   brotli_compress_init, arginfo_brotli_compress_init)
    ZEND_NS_FALIAS(BROTLI_NS, compress_add,
                   brotli_compress_add, arginfo_brotli_compress_add)
    ZEND_NS_FALIAS(BROTLI_NS, uncompress_init,
                   brotli_uncompress_init, arginfo_brotli_uncompress_init)
    ZEND_NS_FALIAS(BROTLI_NS, uncompress_add,
                   brotli_uncompress_add, arginfo_brotli_uncompress_add)
    ZEND_FE_END
};

static const size_t PHP_BROTLI_BUFFER_SIZE = 1 << 19;

struct _php_brotli_context {
    BrotliEncoderState *encoder;
    BrotliDecoderState *decoder;
#if defined(USE_BROTLI_DICTIONARY)
    BrotliEncoderPreparedDictionary *dictionary;
#endif
    size_t available_in;
    const uint8_t *next_in;
    size_t available_out;
    uint8_t *next_out;
    uint8_t *output;
    zend_object std;
};

static void php_brotli_context_init(php_brotli_context *ctx)
{
    ctx->encoder = NULL;
    ctx->decoder = NULL;
#if defined(USE_BROTLI_DICTIONARY)
    ctx->dictionary = NULL;
#endif
    ctx->available_in = 0;
    ctx->next_in = NULL;
    ctx->available_out = 0;
    ctx->next_out = NULL;
    ctx->output = NULL;
}

static int php_brotli_context_create_encoder_ex(php_brotli_context *ctx,
                                                long level,
                                                int lgwin,
                                                long mode,
                                                zend_string *dict,
                                                int fail)
{
    ctx->encoder = BrotliEncoderCreateInstance(NULL, NULL, NULL);
    if (!ctx->encoder) {
        php_error_docref(NULL, E_WARNING, "%sfailed to prepare compression",
                         (fail ? "" : "brotli: "));
        return FAILURE;
    }

    if (level < BROTLI_MIN_QUALITY || level > BROTLI_MAX_QUALITY) {
        php_error_docref(NULL, E_WARNING,
                         "%s compression level (%ld)%s:"
                         " must be within %d..%d",
                         (fail ? "failed to" : "brotli: set"),
                         (long)level,
                         (fail ? "": " to BROTLI_COMPRESS_LEVEL_DEFAULT"),
                         BROTLI_MIN_QUALITY, BROTLI_MAX_QUALITY);
        if (fail) {
            return FAILURE;
        }
        level = BROTLI_DEFAULT_QUALITY;
    }
    if (lgwin == 0) {
        lgwin = BROTLI_DEFAULT_WINDOW;
    }
    if (mode != BROTLI_MODE_GENERIC &&
        mode != BROTLI_MODE_TEXT &&
        mode != BROTLI_MODE_FONT) {
        php_error_docref(NULL, E_WARNING,
                         "%s compression mode (%ld)%s: must be "
                         "BROTLI_GENERIC(%d)|BROTLI_TEXT(%d)|BROTLI_FONT(%d)",
                         (fail ? "failed to" : "brotli: set"),
                         (long)mode,
                         (fail ? "": " to BROTLI_GENERIC"),
                         BROTLI_MODE_GENERIC, BROTLI_MODE_TEXT,
                         BROTLI_MODE_FONT);
        if (fail) {
            return FAILURE;
        }
        mode = BROTLI_MODE_GENERIC;
    }

    if (!BrotliEncoderSetParameter(ctx->encoder, BROTLI_PARAM_QUALITY, level)
        || !BrotliEncoderSetParameter(ctx->encoder, BROTLI_PARAM_LGWIN, lgwin)
        || !BrotliEncoderSetParameter(ctx->encoder, BROTLI_PARAM_MODE, mode)) {
        php_error_docref(NULL, E_WARNING,
                         "%sfailed to set compression parameters",
                         (fail ? "" : "brotli: "));
        return FAILURE;
    }

    if (dict) {
#if defined(USE_BROTLI_DICTIONARY)
        ctx->dictionary
            = BrotliEncoderPrepareDictionary(BROTLI_SHARED_DICTIONARY_RAW,
                                             ZSTR_LEN(dict), ZSTR_VAL(dict),
                                             BROTLI_MAX_QUALITY,
                                             NULL, NULL, NULL);
        if (ctx->dictionary == NULL
            || !BrotliEncoderAttachPreparedDictionary(ctx->encoder,
                                                      ctx->dictionary)) {
            php_error_docref(NULL, E_WARNING,
                             "%sfailed to set compression dictionary",
                             (fail ? "" : "brotli: "));
            return FAILURE;
        }
#else
        php_error_docref(NULL, E_WARNING,
                         "%sfailed to not supported compression dictionary",
                         (fail ? "" : "brotli: "));
        return FAILURE;
#endif
    }

    return SUCCESS;
}

#define php_brotli_context_create_encoder(ctx, level, lgwin, mode) \
    php_brotli_context_create_encoder_ex(ctx, level, lgwin, mode, NULL, 0)

static int php_brotli_context_create_decoder_ex(php_brotli_context *ctx,
                                                zend_string *dict,
                                                int fail)
{
    ctx->decoder = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    if (!ctx->decoder) {
        php_error_docref(NULL, E_WARNING, "%sfailed to prepare uncompression",
                         (fail ? "" : "brotli: "));

        return FAILURE;
    }

    if (!BrotliDecoderSetParameter(ctx->decoder,
                                   BROTLI_DECODER_PARAM_LARGE_WINDOW, 1u)) {
        php_error_docref(NULL, E_WARNING,
                         "%sfailed to set uncompression parameters",
                         (fail ? "" : "brotli: "));
        return FAILURE;
    }

    if (dict) {
#if defined(USE_BROTLI_DICTIONARY)
        if (!BrotliDecoderAttachDictionary(ctx->decoder,
                                           BROTLI_SHARED_DICTIONARY_RAW,
                                           ZSTR_LEN(dict), ZSTR_VAL(dict))) {
            php_error_docref(NULL, E_WARNING,
                             "%sfailed to set uncompression dictionary",
                             (fail ? "" : "brotli: "));
            return FAILURE;
        }
#else
        php_error_docref(NULL, E_WARNING,
                         "%sfailed to not supported uncompression dictionary",
                         (fail ? "" : "brotli: "));
        return FAILURE;
#endif
    }

    return SUCCESS;
}

#define php_brotli_context_create_decoder(ctx) \
    php_brotli_context_create_decoder_ex(ctx, NULL, 0)

static void php_brotli_context_close(php_brotli_context *ctx)
{
    if (ctx->encoder) {
        BrotliEncoderDestroyInstance(ctx->encoder);
        ctx->encoder = NULL;
    }
    if (ctx->decoder) {
        BrotliDecoderDestroyInstance(ctx->decoder);
        ctx->decoder = NULL;
    }
#if defined(USE_BROTLI_DICTIONARY)
    if (ctx->dictionary) {
        BrotliEncoderDestroyPreparedDictionary(ctx->dictionary);
        ctx->dictionary = NULL;
    }
#endif

    if (ctx->output) {
        efree(ctx->output);
        ctx->output = NULL;
    }

    ctx->available_in = 0;
    ctx->next_in = NULL;
    ctx->available_out = 0;
    ctx->next_out = NULL;
}

#define PHP_BROTLI_OUTPUT_HANDLER "ob_brotli_handler"

static int php_brotli_output_encoding(void)
{
#if defined(COMPILE_DL_BROTLI) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    zval *enc;

    if (!BROTLI_G(compression_coding)) {
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
    }

    return BROTLI_G(compression_coding);
}

static int php_brotli_output_handler(void **handler_context,
                                     php_output_context *output_context)
{
    long level = BROTLI_DEFAULT_QUALITY;
    php_brotli_context *ctx = *(php_brotli_context **)handler_context;

    if (!php_brotli_output_encoding()) {
        if ((output_context->op & PHP_OUTPUT_HANDLER_START)
            &&  (output_context->op
                 != (PHP_OUTPUT_HANDLER_START
                     |PHP_OUTPUT_HANDLER_CLEAN|PHP_OUTPUT_HANDLER_FINAL))) {
            sapi_add_header_ex(ZEND_STRL("Vary: Accept-Encoding"),
                               1, 0);
        }
        return FAILURE;
    }

    if (!BROTLI_G(output_compression) || !BROTLI_G(ob_handler)) {
        return FAILURE;
    }

    level = BROTLI_G(output_compression_level);
    if (level < BROTLI_MIN_QUALITY || level > BROTLI_MAX_QUALITY) {
        level = BROTLI_DEFAULT_QUALITY;
    }

    if (output_context->op & PHP_OUTPUT_HANDLER_START) {
        if (php_brotli_context_create_encoder(ctx, level, 0, 0) != SUCCESS) {
            return FAILURE;
        }
    }

    if (!(output_context->op & PHP_OUTPUT_HANDLER_CLEAN)) {
        if (SG(headers_sent)) {
            php_brotli_context_close(ctx);
            return FAILURE;
        }

        if (output_context->in.used) {
            size_t size
                = BrotliEncoderMaxCompressedSize(output_context->in.used);
            if (!ctx->output) {
                ctx->output = (uint8_t *)emalloc(size);
                ctx->available_out = size;
            } else {
                ctx->available_out += size;
                ctx->output = (uint8_t *)erealloc(ctx->output,
                                                  ctx->available_out);
            }

            ctx->next_out = ctx->output;

            // append input
            ctx->available_in = output_context->in.used;
            ctx->next_in = (uint8_t *)output_context->in.data;
        } else {
            ctx->available_in = 0;
            ctx->next_in = NULL;
        }

        if (!BrotliEncoderCompressStream(ctx->encoder,
                                         (output_context->op
                                          & PHP_OUTPUT_HANDLER_FINAL)
                                         ? BROTLI_OPERATION_FINISH
                                         : BROTLI_OPERATION_PROCESS,
                                         &ctx->available_in,
                                         &ctx->next_in,
                                         &ctx->available_out,
                                         &ctx->next_out,
                                         NULL)) {
            php_brotli_context_close(ctx);
            return FAILURE;
        }

        if (output_context->op & PHP_OUTPUT_HANDLER_FINAL) {
            size_t size = (size_t)(ctx->next_out - ctx->output);

            uint8_t *data = (uint8_t *)emalloc(size);
            memcpy(data, ctx->output, size);

            output_context->out.data = data;
            output_context->out.used = size;
            output_context->out.free = 1;

            php_brotli_context_close(ctx);

            sapi_add_header_ex(ZEND_STRL("Content-Encoding: br"),
                               1, 1);
            sapi_add_header_ex(ZEND_STRL("Vary: Accept-Encoding"),
                               1, 0);
        }
    } else {
        php_brotli_context_close(ctx);

        if (output_context->op & PHP_OUTPUT_HANDLER_FINAL) {
            // discard
            return SUCCESS;
        } else {
            // restart
            if (php_brotli_context_create_encoder(ctx,
                                                  level, 0, 0) != SUCCESS) {
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
    php_brotli_context_init(ctx);
    return ctx;
}

static void php_brotli_output_handler_context_dtor(void *opaq)
{
    php_brotli_context *ctx = (php_brotli_context *)opaq;

    if (ctx) {
        php_brotli_context_close(ctx);
        efree(ctx);
        ctx = NULL;
    }

    BROTLI_G(handler_registered) = 0;
    BROTLI_G(ob_handler) = NULL;
}

static php_output_handler*
php_brotli_output_handler_init(const char *handler_name,
                               size_t handler_name_len,
                               size_t chunk_size, int flags)
{
    php_output_handler *handler = NULL;

    handler = php_output_handler_create_internal(handler_name, handler_name_len,
                                                 php_brotli_output_handler,
                                                 chunk_size, flags);
    if (!handler) {
        return NULL;
    }

    if (!BROTLI_G(output_compression)) {
        BROTLI_G(output_compression) = 1;
    }

    BROTLI_G(handler_registered) = 1;

    if (!BROTLI_G(ob_handler)) {
        BROTLI_G(ob_handler) = php_brotli_output_handler_context_init();
    }

    php_output_handler_set_context(handler,
                                   BROTLI_G(ob_handler),
                                   php_brotli_output_handler_context_dtor);

    return handler;
}

static void php_brotli_cleanup_ob_handler_mess(void)
{
    if (BROTLI_G(ob_handler)) {
        php_brotli_output_handler_context_dtor(
            (void *) BROTLI_G(ob_handler)
        );
        BROTLI_G(ob_handler) = NULL;
    }
}

static void php_brotli_output_compression_start(void)
{
    php_output_handler *h;
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
                php_output_handler_start(h);
            }
            break;
    }
}

static PHP_INI_MH(OnUpdate_brotli_output_compression)
{
    int int_value, status;
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

    if (!strncasecmp(ZSTR_VAL(new_value), "off", sizeof("off"))) {
        int_value = 0;
    } else if (!strncasecmp(ZSTR_VAL(new_value), "on", sizeof("on"))) {
        int_value = 1;
#if PHP_VERSION_ID >= 80200
    } else if (zend_ini_parse_quantity_warn(new_value, entry->name)) {
#else
    } else if (zend_atoi(ZSTR_VAL(new_value), ZSTR_LEN(new_value))) {
#endif
        int_value = 1;
    } else {
        int_value = 0;
    }

    if (stage == PHP_INI_STAGE_RUNTIME) {
        status = php_output_get_status();
        if (status & PHP_OUTPUT_SENT) {
            php_error_docref("ref.outcontrol", E_WARNING,
                             "Cannot change brotli.output_compression"
                             " - headers already sent");
            return FAILURE;
        }
    }

    p = (zend_long *)(base+(size_t) mh_arg1);
    *p = int_value;

    BROTLI_G(output_compression) = BROTLI_G(output_compression_default);

    if (stage == PHP_INI_STAGE_RUNTIME && int_value) {
        if (!php_output_handler_started(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER))) {
            php_brotli_output_compression_start();
        }
    }

    return SUCCESS;
}

static int php_brotli_output_conflict(const char *handler_name, size_t handler_name_len)
{
    if (php_output_get_level()) {
        if (php_output_handler_conflict(handler_name, handler_name_len,
                                        ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER))
            || php_output_handler_conflict(handler_name, handler_name_len,
                                           ZEND_STRL("ob_gzhandler"))) {
            return FAILURE;
        }
    }
    return SUCCESS;
}

PHP_INI_BEGIN()
  STD_PHP_INI_BOOLEAN("brotli.output_compression", "0",
                      PHP_INI_ALL, OnUpdate_brotli_output_compression,
                      output_compression_default,
                      zend_brotli_globals, brotli_globals)
  STD_PHP_INI_ENTRY("brotli.output_compression_level", "-1",
                    PHP_INI_ALL, OnUpdateLong, output_compression_level,
                    zend_brotli_globals, brotli_globals)
PHP_INI_END()

static void php_brotli_init_globals(zend_brotli_globals *brotli_globals)
{
    brotli_globals->handler_registered = 0;
    brotli_globals->compression_coding = 0;
    brotli_globals->ob_handler = NULL;
}

typedef struct _php_brotli_stream_data {
    php_brotli_context ctx;
    BrotliDecoderResult result;
    php_stream *stream;
} php_brotli_stream_data;

#define STREAM_DATA_FROM_STREAM() \
    php_brotli_stream_data *self = (php_brotli_stream_data *) stream->abstract

#define STREAM_NAME "compress.brotli"

static int php_brotli_decompress_close(php_stream *stream,
                                       int close_handle)
{
    STREAM_DATA_FROM_STREAM();

    if (!self) {
        return EOF;
    }

    if (close_handle) {
        if (self->stream) {
            php_stream_close(self->stream);
            self->stream = NULL;
        }
    }

    php_brotli_context_close(&self->ctx);

    efree(self);

    stream->abstract = NULL;

    return EOF;
}

#if PHP_VERSION_ID < 70400
static size_t php_brotli_decompress_read(php_stream *stream,
                                         char *buf,
                                         size_t count)
{
    size_t ret = 0;
#else
static ssize_t php_brotli_decompress_read(php_stream *stream,
                                         char *buf,
                                         size_t count)
{
    ssize_t ret = 0;
#endif
    STREAM_DATA_FROM_STREAM();

    /* input */
    uint8_t *input = (uint8_t *)emalloc(PHP_BROTLI_BUFFER_SIZE);
    if (self->result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
        if (php_stream_eof(self->stream)) {
            /* corrupt input */
            if (input) {
                efree(input);
            }
#if PHP_VERSION_ID < 70400
            return 0;
#else
            return -1;
#endif
        }
        self->ctx.available_in = php_stream_read(self->stream, input,
                                                 PHP_BROTLI_BUFFER_SIZE );
        self->ctx.next_in = input;
    }

    /* output */
    uint8_t *output = (uint8_t *)emalloc(count);
    self->ctx.available_out = count;
    self->ctx.next_out = output;

    while (1) {
        self->result = BrotliDecoderDecompressStream(self->ctx.decoder,
                                                     &self->ctx.available_in,
                                                     &self->ctx.next_in,
                                                     &self->ctx.available_out,
                                                     &self->ctx.next_out,
                                                     0);
        if (self->result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT
            || self->result == BROTLI_DECODER_RESULT_SUCCESS) {
            size_t out_size = (size_t)(self->ctx.next_out - output);
            if (out_size) {
                memcpy(buf, output, out_size);
                ret += out_size;
            }
            break;
        } else if (self->result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
            /* more input */
            if (php_stream_eof(self->stream)) {
                /* corrupt input */
                break;
            }
            self->ctx.available_in = php_stream_read(self->stream,
                                                     input, count);
            self->ctx.next_in = input;
        } else {
            /* decoder error */
            break;
        }
    }

    if (input) {
        efree(input);
    }
    if (output) {
        efree(output);
    }

    return ret;
}

static int php_brotli_compress_close(php_stream *stream,
                                     int close_handle)
{
    STREAM_DATA_FROM_STREAM();

    if (!self) {
        return EOF;
    }

    const uint8_t *next_in = NULL;
    size_t available_in = 0;

    uint8_t *output = (uint8_t *)emalloc(PHP_BROTLI_BUFFER_SIZE);

    while (!BrotliEncoderIsFinished(self->ctx.encoder)) {
        uint8_t *next_out = output;
        size_t available_out = PHP_BROTLI_BUFFER_SIZE;
        if (BrotliEncoderCompressStream(self->ctx.encoder,
                                        BROTLI_OPERATION_FINISH,
                                        &available_in,
                                        &next_in,
                                        &available_out,
                                        &next_out,
                                        0)) {
            size_t out_size = (size_t)(next_out - output);
            if (out_size) {
                php_stream_write(self->stream, output, out_size);
            }
        } else {
            php_error_docref(NULL, E_WARNING,
                             "brotli: failed to clean up compression");
        }
    }

    efree(output);

    if (close_handle) {
        if (self->stream) {
            php_stream_close(self->stream);
            self->stream = NULL;
        }
    }

    php_brotli_context_close(&self->ctx);

    efree(self);
    stream->abstract = NULL;

    return EOF;
}

#if PHP_VERSION_ID < 70400
static size_t php_brotli_compress_write(php_stream *stream,
                                        const char *buf,
                                        size_t count)
{
    size_t ret = 0;
#else
static ssize_t php_brotli_compress_write(php_stream *stream,
                                        const char *buf,
                                        size_t count)
{
    ssize_t ret = 0;
#endif
    STREAM_DATA_FROM_STREAM();

    size_t available_in = count;
    const uint8_t *next_in = (uint8_t *)buf;

    uint8_t *output = (uint8_t *)emalloc(PHP_BROTLI_BUFFER_SIZE);

    while (available_in) {
        size_t available_out = PHP_BROTLI_BUFFER_SIZE;
        uint8_t *next_out = output;

        if (BrotliEncoderCompressStream(self->ctx.encoder,
                                        BROTLI_OPERATION_PROCESS,
                                        &available_in,
                                        &next_in,
                                        &available_out,
                                        &next_out,
                                        0)) {
            size_t out_size = (size_t)(next_out - output);
            if (out_size) {
                php_stream_write(self->stream, output, out_size);
            }
        } else {
            php_error_docref(NULL, E_WARNING, "brotli: failed to compression");
#if PHP_VERSION_ID >= 70400
            return -1;
#endif
        }
    }

    ret += count;

    efree(output);

    return ret;
}

static php_stream_ops php_stream_brotli_read_ops = {
    NULL,    /* write */
    php_brotli_decompress_read,
    php_brotli_decompress_close,
    NULL,    /* flush */
    STREAM_NAME,
    NULL,    /* seek */
    NULL,    /* cast */
    NULL,    /* stat */
    NULL     /* set_option */
};

static php_stream_ops php_stream_brotli_write_ops = {
    php_brotli_compress_write,
    NULL,    /* read */
    php_brotli_compress_close,
    NULL,    /* flush */
    STREAM_NAME,
    NULL,    /* seek */
    NULL,    /* cast */
    NULL,    /* stat */
    NULL     /* set_option */
};

static php_stream *
php_stream_brotli_opener(
    php_stream_wrapper *wrapper,
    const char *path,
    const char *mode,
    int options,
    zend_string **opened_path,
    php_stream_context *context
    STREAMS_DC)
{
    php_brotli_stream_data *self;
    int level = BROTLI_DEFAULT_QUALITY;
    int compress;

    if (strncasecmp(STREAM_NAME, path, sizeof(STREAM_NAME)-1) == 0) {
        path += sizeof(STREAM_NAME)-1;
        if (strncmp("://", path, 3) == 0) {
            path += 3;
        }
    }

    if (php_check_open_basedir(path)) {
        return NULL;
    }

    if (!strcmp(mode, "w") || !strcmp(mode, "wb")) {
       compress = 1;
    } else if (!strcmp(mode, "r") || !strcmp(mode, "rb")) {
       compress = 0;
    } else {
        php_error_docref(NULL, E_ERROR, "brotli: invalid open mode");
        return NULL;
    }

    if (context) {
        zval *tmpzval;

        if (NULL != (tmpzval = php_stream_context_get_option(
                         context, "brotli", "level"))) {
            level = zval_get_long(tmpzval);
        }
    }
    if (level > BROTLI_MAX_QUALITY) {
        php_error_docref(NULL, E_WARNING,
                         "brotli: set compression level (%d)"
                         " to BROTLI_COMPRESS_LEVEL_MAX:"
                         " must be within %d..%d",
                         level, BROTLI_MIN_QUALITY, BROTLI_MAX_QUALITY);
        level = BROTLI_MAX_QUALITY;
    } else if (level <  BROTLI_MIN_QUALITY) {
        php_error_docref(NULL, E_WARNING,
                         "brotli: set compression level (%d)"
                         " to BROTLI_COMPRESS_LEVEL_DEFAULT:"
                         " must be within %d..%d",
                         level, BROTLI_MIN_QUALITY, BROTLI_MAX_QUALITY);
        level = BROTLI_DEFAULT_QUALITY;
    }

    self = ecalloc(sizeof(*self), 1);
    self->stream = php_stream_open_wrapper(path, mode,
                                           options | REPORT_ERRORS, NULL);
    if (!self->stream) {
        efree(self);
        return NULL;
    }

    php_brotli_context_init(&self->ctx);

    /* File */
    if (compress) {
        if (php_brotli_context_create_encoder(&self->ctx,
                                              level, 0, 0) != SUCCESS) {
            php_error_docref(NULL, E_WARNING,
                             "brotli: failed to prepare compression");
            php_stream_close(self->stream);
            efree(self);
            return NULL;
        }

        return php_stream_alloc(&php_stream_brotli_write_ops, self, NULL, mode);
    } else {
        if (php_brotli_context_create_decoder(&self->ctx) != SUCCESS) {
            php_error_docref(NULL, E_WARNING,
                             "brotli: failed to prepare decompression");
            php_stream_close(self->stream);
            efree(self);
            return NULL;
        }

        self->result = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;

        return php_stream_alloc(&php_stream_brotli_read_ops, self, NULL, mode);
    }
    return NULL;
}

static php_stream_wrapper_ops brotli_stream_wops = {
    php_stream_brotli_opener,
    NULL,    /* close */
    NULL,    /* fstat */
    NULL,    /* stat */
    NULL,    /* opendir */
    STREAM_NAME,
    NULL,    /* unlink */
    NULL,    /* rename */
    NULL,    /* mkdir */
    NULL,    /* rmdir */
    NULL
};

php_stream_wrapper php_stream_brotli_wrapper = {
    &brotli_stream_wops,
    NULL,
    0 /* is_url */
};

/* Brotli Compress/UnCompress Context */
#define PHP_BROTLI_CONTEXT_OBJ_INIT_OF_CLASS(ce) \
  php_brotli_context *ctx; \
  object_init_ex(return_value, ce); \
  ctx = php_brotli_context_from_obj(Z_OBJ_P(return_value)); \
  php_brotli_context_init(ctx)

static php_brotli_context *php_brotli_context_from_obj(zend_object *obj)
{
    return (php_brotli_context *)
        ((char *)(obj) - XtOffsetOf(php_brotli_context, std));
}

static void php_brotli_context_free_obj(zend_object *object)
{
    php_brotli_context *intern = php_brotli_context_from_obj(object);
    php_brotli_context_close(intern);
    zend_object_std_dtor(&intern->std);
}

static zend_object *
php_brotli_context_create_object(zend_class_entry *class_type,
                                 zend_object_handlers *handlers)
{
    php_brotli_context *intern;
#if PHP_VERSION_ID >= 80000
    intern = zend_object_alloc(sizeof(php_brotli_context), class_type);
#else
    intern = ecalloc(1,
                     sizeof(php_brotli_context)
                     + zend_object_properties_size(class_type));
#endif
    zend_object_std_init(&intern->std, class_type);
    object_properties_init(&intern->std, class_type);
    intern->std.handlers = handlers;

    return &intern->std;
}

/* Brotli Compress Context */
zend_class_entry *php_brotli_compress_context_ce;
static zend_object_handlers php_brotli_compress_context_object_handlers;

static zend_object *
php_brotli_compress_context_create_object(zend_class_entry *class_type)
{
    return php_brotli_context_create_object(
        class_type,
        &php_brotli_compress_context_object_handlers);
}

static zend_function *
php_brotli_compress_context_get_constructor(zend_object *object)
{
    zend_throw_error(NULL,
                     "Cannot directly construct Brotli\\Compress\\Context, "
                     "use brotli_compress_init() instead");
    return NULL;
}

static zend_class_entry *php_brotli_compress_context_register_class(void)
{
    zend_class_entry ce, *class_entry;

    INIT_NS_CLASS_ENTRY(ce, "Brotli\\Compress", "Context", NULL);
#if PHP_VERSION_ID >= 80000
    class_entry = zend_register_internal_class_ex(&ce, NULL);
#if PHP_VERSION_ID >= 80100
    class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE;
#else
    class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
#endif
#else
    ce.create_object = php_brotli_compress_context_create_object;
    class_entry = zend_register_internal_class(&ce);
    class_entry->ce_flags |= ZEND_ACC_FINAL;
#endif

    return class_entry;
}

/* Brotli UnCompress Context */
zend_class_entry *php_brotli_uncompress_context_ce;
static zend_object_handlers php_brotli_uncompress_context_object_handlers;

static zend_object *
php_brotli_uncompress_context_create_object(zend_class_entry *class_type)
{
    return php_brotli_context_create_object(
        class_type,
        &php_brotli_uncompress_context_object_handlers);
}

static zend_function *
php_brotli_uncompress_context_get_constructor(zend_object *object)
{
  zend_throw_error(NULL,
                   "Cannot directly construct Brotli\\UnCompress\\Context, "
                   "use brotli_uncompress_init() instead");
  return NULL;
}

static zend_class_entry *php_brotli_uncompress_context_register_class(void)
{
    zend_class_entry ce, *class_entry;

    INIT_NS_CLASS_ENTRY(ce, "Brotli\\UnCompress", "Context", NULL);
#if PHP_VERSION_ID >= 80000
    class_entry = zend_register_internal_class_ex(&ce, NULL);
#if PHP_VERSION_ID >= 80100
    class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE;
#else
    class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
#endif
#else
    ce.create_object = php_brotli_uncompress_context_create_object;
    class_entry = zend_register_internal_class(&ce);
    class_entry->ce_flags |= ZEND_ACC_FINAL;
#endif

    return class_entry;
}

ZEND_MINIT_FUNCTION(brotli)
{
    ZEND_INIT_MODULE_GLOBALS(brotli, php_brotli_init_globals, NULL);

    REGISTER_LONG_CONSTANT("BROTLI_GENERIC", BROTLI_MODE_GENERIC,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_TEXT", BROTLI_MODE_TEXT,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_FONT", BROTLI_MODE_FONT,
                           CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("BROTLI_COMPRESS_LEVEL_MIN", BROTLI_MIN_QUALITY,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_COMPRESS_LEVEL_MAX", BROTLI_MAX_QUALITY,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_COMPRESS_LEVEL_DEFAULT",
                           BROTLI_DEFAULT_QUALITY,
                           CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("BROTLI_PROCESS", BROTLI_OPERATION_PROCESS,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_FLUSH", BROTLI_OPERATION_FLUSH,
                           CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BROTLI_FINISH", BROTLI_OPERATION_FINISH,
                           CONST_CS | CONST_PERSISTENT);

#if defined(USE_BROTLI_DICTIONARY)
    REGISTER_BOOL_CONSTANT("BROTLI_DICTIONARY_SUPPORT", 1,
                           CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_BOOL_CONSTANT("BROTLI_DICTIONARY_SUPPORT", 0,
                           CONST_CS | CONST_PERSISTENT);
#endif

    php_output_handler_alias_register(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER),
                                      php_brotli_output_handler_init);
    php_output_handler_conflict_register(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER),
                                         php_brotli_output_conflict);

    php_brotli_compress_context_ce
        = php_brotli_compress_context_register_class();
#if PHP_VERSION_ID >= 80000
    php_brotli_compress_context_ce->create_object
        = php_brotli_compress_context_create_object;
#if PHP_VERSION_ID >= 80300
    php_brotli_compress_context_ce->default_object_handlers
        = &php_brotli_compress_context_object_handlers;
#endif
#if PHP_VERSION_ID < 80100
    php_brotli_compress_context_ce->serialize = zend_class_serialize_deny;
    php_brotli_compress_context_ce->unserialize = zend_class_unserialize_deny;
#endif
#endif
    memcpy(&php_brotli_compress_context_object_handlers,
           &std_object_handlers, sizeof(zend_object_handlers));
    php_brotli_compress_context_object_handlers.offset
        = XtOffsetOf(php_brotli_context, std);
    php_brotli_compress_context_object_handlers.free_obj
        = php_brotli_context_free_obj;
    php_brotli_compress_context_object_handlers.get_constructor
        = php_brotli_compress_context_get_constructor;
    php_brotli_compress_context_object_handlers.clone_obj = NULL;
#if PHP_VERSION_ID >= 80000
    php_brotli_compress_context_object_handlers.compare
        = zend_objects_not_comparable;
#endif

    php_brotli_uncompress_context_ce
        = php_brotli_uncompress_context_register_class();
#if PHP_VERSION_ID >= 80000
    php_brotli_uncompress_context_ce->create_object
        = php_brotli_uncompress_context_create_object;
#if PHP_VERSION_ID >= 80300
    php_brotli_uncompress_context_ce->default_object_handlers
        = &php_brotli_uncompress_context_object_handlers;
#endif
#if PHP_VERSION_ID < 80100
    php_brotli_uncompress_context_ce->serialize = zend_class_serialize_deny;
    php_brotli_uncompress_context_ce->unserialize = zend_class_unserialize_deny;
#endif
#endif
    memcpy(&php_brotli_uncompress_context_object_handlers,
           &std_object_handlers, sizeof(zend_object_handlers));
    php_brotli_uncompress_context_object_handlers.offset
        = XtOffsetOf(php_brotli_context, std);
    php_brotli_uncompress_context_object_handlers.free_obj
        = php_brotli_context_free_obj;
    php_brotli_uncompress_context_object_handlers.get_constructor
        = php_brotli_uncompress_context_get_constructor;
    php_brotli_uncompress_context_object_handlers.clone_obj = NULL;
#if PHP_VERSION_ID >= 80000
    php_brotli_uncompress_context_object_handlers.compare
        = zend_objects_not_comparable;
#endif

    REGISTER_INI_ENTRIES();

    php_register_url_stream_wrapper(STREAM_NAME,
                                    &php_stream_brotli_wrapper);

#if defined(HAVE_APCU_SUPPORT)
    apc_register_serializer("brotli",
                            APC_SERIALIZER_NAME(brotli),
                            APC_UNSERIALIZER_NAME(brotli),
                            NULL);
#endif

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(brotli)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

ZEND_RINIT_FUNCTION(brotli)
{
    BROTLI_G(compression_coding) = 0;
    if (!BROTLI_G(handler_registered)) {
        BROTLI_G(output_compression) = BROTLI_G(output_compression_default);
        php_brotli_output_compression_start();
    }
    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(brotli)
{
    php_brotli_cleanup_ob_handler_mess();
    BROTLI_G(handler_registered) = 0;
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(brotli)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Brotli support", "enabled");
    php_info_print_table_row(2, "Extension Version", BROTLI_EXT_VERSION);
#ifdef BROTLI_LIB_VERSION
    php_info_print_table_row(2, "Library Version", BROTLI_LIB_VERSION);
#else
    uint32_t version = BrotliDecoderVersion();
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d.%d.%d",
             version >> 24, (version >> 12) & 0xfff, version & 0xfff);
    php_info_print_table_row(2, "Library Version", buffer);
#endif
#if defined(USE_BROTLI_DICTIONARY)
    php_info_print_table_row(2, "Dictionary support", "enabled");
#else
    php_info_print_table_row(2, "Dictionary support", "disabled");
#endif
#if defined(HAVE_APCU_SUPPORT)
    php_info_print_table_row(2, "APCu serializer ABI", APC_SERIALIZER_ABI);
#endif
    php_info_print_table_end();
}

#if defined(HAVE_APCU_SUPPORT)
static const zend_module_dep brotli_module_deps[] = {
    ZEND_MOD_OPTIONAL("apcu")
    ZEND_MOD_END
};
#endif

zend_module_entry brotli_module_entry = {
#if defined(HAVE_APCU_SUPPORT)
    STANDARD_MODULE_HEADER_EX,
    NULL,
    brotli_module_deps,
#else
    STANDARD_MODULE_HEADER,
#endif
    "brotli",
    brotli_functions,
    ZEND_MINIT(brotli),
    ZEND_MSHUTDOWN(brotli),
    ZEND_RINIT(brotli),
    ZEND_RSHUTDOWN(brotli),
    ZEND_MINFO(brotli),
    BROTLI_EXT_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BROTLI
#if defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(brotli)
#endif

static ZEND_FUNCTION(brotli_compress)
{
    char *in;
    size_t in_size;

    zend_long level = BROTLI_DEFAULT_QUALITY;
    zend_long mode =  BROTLI_MODE_GENERIC;
    zend_string *dict = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 4)
        Z_PARAM_STRING(in, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(level)
        Z_PARAM_LONG(mode)
        Z_PARAM_STR_OR_NULL(dict)
    ZEND_PARSE_PARAMETERS_END();

    php_brotli_context ctx;
    php_brotli_context_init(&ctx);
    if (php_brotli_context_create_encoder_ex(&ctx, level, 0, mode,
                                             dict, 1) != SUCCESS) {
        php_brotli_context_close(&ctx);
        RETURN_FALSE;
    }

    uint8_t *buffer;
    size_t buffer_size;
    buffer_size = BrotliEncoderMaxCompressedSize(in_size);
    buffer_size = (buffer_size < 64) ? 64 : buffer_size;
    buffer = (uint8_t *)emalloc(buffer_size);

    ctx.next_in = (const uint8_t *)in;
    ctx.available_in = in_size;
    smart_string out = {0};
    while (!BrotliEncoderIsFinished(ctx.encoder)) {
        ctx.available_out = buffer_size;
        ctx.next_out = buffer;
        if (BrotliEncoderCompressStream(ctx.encoder, BROTLI_OPERATION_FINISH,
                                        &ctx.available_in, &ctx.next_in,
                                        &ctx.available_out, &ctx.next_out,
                                        0)) {
            size_t used_out = (size_t)(ctx.next_out - buffer);
            if (used_out) {
                smart_string_appendl(&out, buffer, used_out);
            }
        } else {
            efree(buffer);
            smart_string_free(&out);
            php_error_docref(NULL, E_WARNING, "failed to compress");
            RETURN_FALSE;
        }
    }

    php_brotli_context_close(&ctx);

    RETVAL_STRINGL(out.c, out.len);

    efree(buffer);
    smart_string_free(&out);
}

static ZEND_FUNCTION(brotli_compress_init)
{
    zend_long level = BROTLI_DEFAULT_QUALITY;
    zend_long mode =  BROTLI_MODE_GENERIC;
    zend_string *dict = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 3)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(level)
        Z_PARAM_LONG(mode)
        Z_PARAM_STR_OR_NULL(dict)
    ZEND_PARSE_PARAMETERS_END();

    PHP_BROTLI_CONTEXT_OBJ_INIT_OF_CLASS(php_brotli_compress_context_ce);

    if (php_brotli_context_create_encoder_ex(ctx, level, 0, mode,
                                             dict, 1) != SUCCESS) {
        zval_ptr_dtor(return_value);
        RETURN_FALSE;
    }
}

static ZEND_FUNCTION(brotli_compress_add)
{
    zval *res;
    php_brotli_context *ctx;
    size_t buffer_size, buffer_used;
    zend_long mode = BROTLI_OPERATION_FLUSH;
#if PHP_VERSION_ID >= 80000
    zend_object *obj;
#else
    zval *obj;
#endif
    char *in_buf;
    size_t in_size;
    smart_string out = {0};

    ZEND_PARSE_PARAMETERS_START(2, 3)
#if PHP_VERSION_ID >= 80000
        Z_PARAM_OBJ_OF_CLASS(obj, php_brotli_compress_context_ce)
#else
        Z_PARAM_OBJECT_OF_CLASS(obj, php_brotli_compress_context_ce)
#endif
        Z_PARAM_STRING(in_buf, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    if (mode != BROTLI_OPERATION_PROCESS &&
        mode != BROTLI_OPERATION_FLUSH &&
        mode != BROTLI_OPERATION_FINISH) {
        php_error_docref(NULL, E_WARNING,
                         "failed to compression mode (%ld):"
                         " must be BROTLI_PROCESS(%d)|"
                         "BROTLI_FLUSH(%d)|BROTLI_FINISH(%d)",
                         (long)mode, BROTLI_OPERATION_PROCESS,
                         BROTLI_OPERATION_FLUSH, BROTLI_OPERATION_FINISH);
        RETURN_FALSE;
    }

#if PHP_VERSION_ID >= 80000
    ctx = php_brotli_context_from_obj(obj);
#else
    ctx = php_brotli_context_from_obj(Z_OBJ_P(obj));
#endif
    if (ctx == NULL || ctx->encoder == NULL) {
        php_error_docref(NULL, E_WARNING,
                         "failed to prepare incremental compress");
        RETURN_FALSE;
    }

    buffer_size = BrotliEncoderMaxCompressedSize(in_size);
    buffer_size = (buffer_size < 64) ? 64 : buffer_size;
    uint8_t *buffer = (uint8_t *)emalloc(buffer_size);

    ctx->next_in = in_buf;
    ctx->available_in = in_size;

    while (ctx->available_in || BrotliEncoderHasMoreOutput(ctx->encoder)) {
        ctx->available_out = buffer_size;
        ctx->next_out = buffer;
        if (BrotliEncoderCompressStream(ctx->encoder,
                                        mode,
                                        &ctx->available_in,
                                        &ctx->next_in,
                                        &ctx->available_out,
                                        &ctx->next_out,
                                        0)) {
            buffer_used = (size_t)(ctx->next_out - buffer);
            if (buffer_used) {
                smart_string_appendl(&out, buffer, buffer_used);
            }
        } else {
            efree(buffer);
            smart_string_free(&out);
            php_error_docref(NULL, E_WARNING,
                             "failed to incremental compress");
            RETURN_FALSE;
        }
    }

    if (mode == BROTLI_OPERATION_FINISH) {
        while (!BrotliEncoderIsFinished(ctx->encoder)) {
            ctx->available_out = buffer_size;
            ctx->next_out = buffer;
            if (BrotliEncoderCompressStream(ctx->encoder,
                                            BROTLI_OPERATION_FINISH,
                                            &ctx->available_in,
                                            &ctx->next_in,
                                            &ctx->available_out,
                                            &ctx->next_out,
                                            0)) {
                buffer_used = (size_t)(ctx->next_out - buffer);
                if (buffer_used) {
                    smart_string_appendl(&out, buffer, buffer_used);
                }
            } else {
                efree(buffer);
                smart_string_free(&out);
                php_error_docref(NULL, E_WARNING,
                                 "failed to incremental compress");
                RETURN_FALSE;
            }
        }
    }

    RETVAL_STRINGL(out.c, out.len);

    efree(buffer);
    smart_string_free(&out);
}

static ZEND_FUNCTION(brotli_uncompress)
{
    zend_long max_size = 0;
    char *in;
    size_t in_size;
    smart_string out = {0};
    zend_string *dict = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_STRING(in, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(max_size)
        Z_PARAM_STR_OR_NULL(dict)
    ZEND_PARSE_PARAMETERS_END();

    if (max_size && max_size < in_size) {
        in_size = max_size;
    }

    php_brotli_context ctx;
    php_brotli_context_init(&ctx);
    if (php_brotli_context_create_decoder_ex(&ctx, dict, 1) != SUCCESS) {
        php_brotli_context_close(&ctx);
        RETURN_FALSE;
    }

    ctx.available_in = in_size;
    ctx.next_in = (const uint8_t *)in;
    size_t buffer_size = PHP_BROTLI_BUFFER_SIZE;
    uint8_t *buffer = (uint8_t *)emalloc(buffer_size);
    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
    while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
        ctx.available_out = buffer_size;
        ctx.next_out = buffer;
        result = BrotliDecoderDecompressStream(ctx.decoder,
                                               &ctx.available_in,
                                               &ctx.next_in,
                                               &ctx.available_out,
                                               &ctx.next_out,
                                               0);
        size_t used_out = (size_t)(buffer_size - ctx.available_out);
        if (used_out != 0) {
            smart_string_appendl(&out, buffer, used_out);
        }
    }

    php_brotli_context_close(&ctx);
    efree(buffer);

    if (result != BROTLI_DECODER_RESULT_SUCCESS) {
        php_error_docref(NULL, E_WARNING, "failed to uncompress");
        smart_string_free(&out);
        RETURN_FALSE;
    }

    RETVAL_STRINGL(out.c, out.len);
    smart_string_free(&out);
}

static ZEND_FUNCTION(brotli_uncompress_init)
{
    zend_string *dict = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(dict)
    ZEND_PARSE_PARAMETERS_END();

    PHP_BROTLI_CONTEXT_OBJ_INIT_OF_CLASS(php_brotli_uncompress_context_ce);

    if (php_brotli_context_create_decoder_ex(ctx, dict, 1) != SUCCESS) {
        zval_ptr_dtor(return_value);
        RETURN_FALSE;
    }
}

static ZEND_FUNCTION(brotli_uncompress_add)
{
    zval *res;
    php_brotli_context *ctx;
    size_t buffer_size;
    zend_long mode = BROTLI_OPERATION_FLUSH;
#if PHP_VERSION_ID >= 80000
    zend_object *obj;
#else
    zval *obj;
#endif
    char *in_buf;
    size_t in_size;
    smart_string out = {0};

    ZEND_PARSE_PARAMETERS_START(2, 3)
#if PHP_VERSION_ID >= 80000
        Z_PARAM_OBJ_OF_CLASS(obj, php_brotli_uncompress_context_ce)
#else
        Z_PARAM_OBJECT_OF_CLASS(obj, php_brotli_uncompress_context_ce)
#endif
        Z_PARAM_STRING(in_buf, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    if (mode != BROTLI_OPERATION_PROCESS &&
        mode != BROTLI_OPERATION_FLUSH &&
        mode != BROTLI_OPERATION_FINISH) {
        php_error_docref(NULL, E_WARNING,
                         "mode (%ld) must be BROTLI_PROCESS(%d)"
                         "|BROTLI_FLUSH(%d)|BROTLI_FINISH(%d)",
                         (long)mode, BROTLI_OPERATION_PROCESS,
                         BROTLI_OPERATION_FLUSH, BROTLI_OPERATION_FINISH);
        RETURN_FALSE;
    }

#if PHP_VERSION_ID >= 80000
    ctx = php_brotli_context_from_obj(obj);
#else
    ctx = php_brotli_context_from_obj(Z_OBJ_P(obj));
#endif
    if (ctx == NULL || ctx->decoder == NULL) {
        php_error_docref(NULL, E_WARNING,
                         "failed to prepare incremental uncompress");
        RETURN_FALSE;
    }

    if (in_size <= 0 && mode != BROTLI_OPERATION_FINISH) {
        RETURN_EMPTY_STRING();
    }

    buffer_size = PHP_BROTLI_BUFFER_SIZE;
    uint8_t *buffer = (uint8_t *)emalloc(buffer_size);

    ctx->next_in = (const uint8_t *)in_buf;
    ctx->available_in = in_size;

    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
    while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
        ctx->available_out = buffer_size;
        ctx->next_out = buffer;
        result = BrotliDecoderDecompressStream(ctx->decoder,
                                               &ctx->available_in,
                                               &ctx->next_in,
                                               &ctx->available_out,
                                               &ctx->next_out,
                                               0);
        size_t buffer_used = buffer_size - ctx->available_out;
        if (buffer_used) {
            smart_string_appendl(&out, buffer, buffer_used);
        }
    }

    RETVAL_STRINGL(out.c, out.len);

    efree(buffer);
    smart_string_free(&out);
}

#if defined(HAVE_APCU_SUPPORT)
static int APC_SERIALIZER_NAME(brotli)(APC_SERIALIZER_ARGS)
{
    int result;
    int lgwin = BROTLI_DEFAULT_WINDOW, level = BROTLI_DEFAULT_QUALITY;
    php_serialize_data_t var_hash;
    smart_str var = {0};
    BrotliEncoderMode mode = BROTLI_MODE_GENERIC;

    PHP_VAR_SERIALIZE_INIT(var_hash);
    php_var_serialize(&var, (zval*) value, &var_hash);
    PHP_VAR_SERIALIZE_DESTROY(var_hash);
    if (var.s == NULL) {
        return 0;
    }

    *buf_len = BrotliEncoderMaxCompressedSize(ZSTR_LEN(var.s));
    *buf = (char*) emalloc(*buf_len);

    if (!BrotliEncoderCompress(level, lgwin, mode,
                               ZSTR_LEN(var.s),
                               (const uint8_t*) ZSTR_VAL(var.s),
                               buf_len, (uint8_t*) *buf)) {
        efree(*buf);
        *buf = NULL;
        *buf_len = 0;
        result = 0;
    } else {
        result = 1;
    }

    smart_str_free(&var);

    return result;
}

static int APC_UNSERIALIZER_NAME(brotli)(APC_UNSERIALIZER_ARGS)
{
    const uint8_t *next_in = (const uint8_t*) buf;
    const unsigned char* tmp;
    int result;
    php_unserialize_data_t var_hash;
    size_t available_in = buf_len;
    size_t available_out, used_out;
    smart_str out = {NULL, 0};
    uint8_t *var, *next_out;
    BrotliDecoderResult res = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
    BrotliDecoderState *state;

    state = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    if (!state) {
        ZVAL_NULL(value);
        return 0;
    }

    var = (uint8_t*) emalloc(PHP_BROTLI_BUFFER_SIZE);

    while (res == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
        available_out = PHP_BROTLI_BUFFER_SIZE;
        next_out = var;
        res = BrotliDecoderDecompressStream(state, &available_in, &next_in,
                                            &available_out, &next_out,
                                            0);
        used_out = PHP_BROTLI_BUFFER_SIZE - available_out;
        if (used_out != 0) {
            smart_str_appendl(&out, var, used_out);
        }
    }

    BrotliDecoderDestroyInstance(state);
    efree(var);

    if (ZSTR_LEN(out.s) <= 0) {
        smart_str_free(&out);
        ZVAL_NULL(value);
        return 0;
    }

    PHP_VAR_UNSERIALIZE_INIT(var_hash);
    tmp = ZSTR_VAL(out.s);
    result = php_var_unserialize(value, &tmp,
                                 ZSTR_VAL(out.s) + ZSTR_LEN(out.s),
                                 &var_hash);
    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

    if (!result) {
        php_error_docref(NULL, E_NOTICE,
                         "Error at offset " ZEND_LONG_FMT
                         " of " ZEND_LONG_FMT " bytes",
                         (zend_long) ((char*) tmp - ZSTR_VAL(out.s)),
                         (zend_long) ZSTR_LEN(out.s));
        ZVAL_NULL(value);
        result = 0;
    } else {
        result = 1;
    }

    smart_str_free(&out);

    return result;
}
#endif
