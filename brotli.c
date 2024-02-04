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
#include "php_brotli.h"

int le_state;

# pragma GCC diagnostic ignored "-Wpointer-sign"

ZEND_DECLARE_MODULE_GLOBALS(brotli);

static ZEND_FUNCTION(brotli_compress);
static ZEND_FUNCTION(brotli_uncompress);
static ZEND_FUNCTION(brotli_compress_init);
static ZEND_FUNCTION(brotli_compress_add);
static ZEND_FUNCTION(brotli_uncompress_init);
static ZEND_FUNCTION(brotli_uncompress_add);

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_compress, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, quality)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_uncompress, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_compress_init, 0, 0, 0)
    ZEND_ARG_INFO(0, quality)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_compress_add, 0, 0, 2)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_uncompress_init, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_brotli_uncompress_add, 0, 0, 2)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

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

static int php_brotli_encoder_create(BrotliEncoderState **encoder,
                                     long quality, int lgwin, long mode)
{
    *encoder = BrotliEncoderCreateInstance(NULL, NULL, NULL);
    if (!*encoder) {
        return FAILURE;
    }

    if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) {
        php_error_docref(NULL, E_WARNING,
                         "brotli: compression level (%ld) "
                         "must be within %d..%d",
                         (long)quality, BROTLI_MIN_QUALITY, BROTLI_MAX_QUALITY);
        quality = BROTLI_DEFAULT_QUALITY;
    }
    if (lgwin == 0) {
        lgwin = BROTLI_DEFAULT_WINDOW;
    }
    if (mode != BROTLI_MODE_GENERIC &&
        mode != BROTLI_MODE_TEXT &&
        mode != BROTLI_MODE_FONT) {
        php_error_docref(NULL, E_WARNING,
                         "brotli: compression mode (%ld) must be %d, %d, %d",
                         (long)mode, BROTLI_MODE_GENERIC, BROTLI_MODE_TEXT,
                         BROTLI_MODE_FONT);
        mode = BROTLI_MODE_GENERIC;
    }

    BrotliEncoderSetParameter(*encoder, BROTLI_PARAM_QUALITY, quality);
    BrotliEncoderSetParameter(*encoder, BROTLI_PARAM_LGWIN, lgwin);
    BrotliEncoderSetParameter(*encoder, BROTLI_PARAM_MODE, mode);

    return SUCCESS;
}

static int php_brotli_decoder_create(BrotliDecoderState **decoder)
{
    *decoder = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    if (!*decoder) {
        return FAILURE;
    }

    return SUCCESS;
}

static php_brotli_state_context* php_brotli_state_init(void)
{
    php_brotli_state_context *ctx
        = (php_brotli_state_context *)ecalloc(1,
                                              sizeof(php_brotli_state_context));
    ctx->encoder = NULL;
    ctx->decoder = NULL;
    return ctx;
}

static void php_brotli_state_destroy(php_brotli_state_context *ctx)
{
    if (ctx->encoder) {
        BrotliEncoderDestroyInstance(ctx->encoder);
        ctx->encoder = NULL;
    }
    if (ctx->decoder) {
        BrotliDecoderDestroyInstance(ctx->decoder);
        ctx->decoder = NULL;
    }
    efree(ctx);
}

static void php_brotli_state_rsrc_dtor(zend_resource *res)
{
    php_brotli_state_context *ctx = zend_fetch_resource(res, NULL, le_state);
    php_brotli_state_destroy(ctx);
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

static void php_brotli_context_close(php_brotli_context *ctx)
{
    if (ctx->state.encoder) {
        BrotliEncoderDestroyInstance(ctx->state.encoder);
        ctx->state.encoder = NULL;
    }
    if (ctx->output) {
        efree(ctx->output);
        ctx->output = NULL;
    }

    ctx->available_in = 0;
    ctx->next_in = NULL;
    ctx->available_out = 0;
    ctx->next_out = NULL;
}

static int php_brotli_output_handler(void **handler_context,
                                     php_output_context *output_context)
{
    long quality = BROTLI_DEFAULT_QUALITY;
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

    quality = BROTLI_G(output_compression_level);
    if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) {
        quality = BROTLI_DEFAULT_QUALITY;
    }

    if (output_context->op & PHP_OUTPUT_HANDLER_START) {
        if (php_brotli_encoder_create(&ctx->state.encoder,
                                      quality, 0, 0) != SUCCESS) {
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

        if (!BrotliEncoderCompressStream(ctx->state.encoder,
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
            if (php_brotli_encoder_create(&ctx->state.encoder,
                                          quality, 0, 0) != SUCCESS) {
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
    ctx->state.encoder = NULL;
    ctx->state.decoder = NULL;
    ctx->available_in = 0;
    ctx->next_in = NULL;
    ctx->available_out = 0;
    ctx->next_out = NULL;
    ctx->output = NULL;
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

    BROTLI_G(handler_registered) = 1;

    if (!BROTLI_G(ob_handler)) {
        BROTLI_G(ob_handler) = php_brotli_output_handler_context_init();
    }

    php_output_handler_set_context(handler,
                                   BROTLI_G(ob_handler),
                                   php_brotli_output_handler_context_dtor);

    BROTLI_G(output_compression) = 1;

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
    brotli_globals->ob_handler = NULL;
}

typedef struct _php_brotli_stream_data {
    BrotliEncoderState *cctx;
    BrotliDecoderState *dctx;
    BrotliDecoderResult result;
    size_t available_in;
    const uint8_t *next_in;
    size_t available_out;
    uint8_t *next_out;
    uint8_t *output;
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

    if (self->dctx) {
        BrotliDecoderDestroyInstance(self->dctx);
        self->dctx = NULL;
    }
    if (self->output) {
        efree(self->output);
    }
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
        self->available_in = php_stream_read(self->stream, input,
                                             PHP_BROTLI_BUFFER_SIZE );
        self->next_in = input;
    }

    /* output */
    uint8_t *output = (uint8_t *)emalloc(count);
    self->available_out = count;
    self->next_out = output;

    while (1) {
        self->result = BrotliDecoderDecompressStream(self->dctx,
                                                     &self->available_in,
                                                     &self->next_in,
                                                     &self->available_out,
                                                     &self->next_out,
                                                     0);
        if (self->result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT
            || self->result == BROTLI_DECODER_RESULT_SUCCESS) {
            size_t out_size = (size_t)(self->next_out - output);
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
            self->available_in = php_stream_read(self->stream, input, count);
            self->next_in = input;
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

    while (!BrotliEncoderIsFinished(self->cctx)) {
        uint8_t *next_out = output;
        size_t available_out = PHP_BROTLI_BUFFER_SIZE;
        if (BrotliEncoderCompressStream(self->cctx,
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
                             "brotli compress error");
        }
    }

    efree(output);

    if (close_handle) {
        if (self->stream) {
            php_stream_close(self->stream);
            self->stream = NULL;
        }
    }

    if (self->cctx) {
        BrotliEncoderDestroyInstance(self->cctx);
        self->cctx = NULL;
    }
    if (self->output) {
        efree(self->output);
    }
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

        if (BrotliEncoderCompressStream(self->cctx,
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
            php_error_docref(NULL, E_WARNING, "brotli compress error");
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
                         "brotli: compression level (%d) must be less than %d",
                         level, BROTLI_MAX_QUALITY);
        level = BROTLI_MAX_QUALITY;
    }

    self = ecalloc(sizeof(*self), 1);
    self->stream = php_stream_open_wrapper(path, mode,
                                           options | REPORT_ERRORS, NULL);
    if (!self->stream) {
        efree(self);
        return NULL;
    }

    /* File */
    if (compress) {
        self->dctx = NULL;
        if (php_brotli_encoder_create(&self->cctx, level, 0, 0) != SUCCESS) {
            php_error_docref(NULL, E_WARNING,
                             "brotli: compression context failed");
            php_stream_close(self->stream);
            efree(self);
            return NULL;
        }
        self->available_in = 0;
        self->next_in = NULL;
        self->available_out = 0;
        self->next_out = NULL;
        self->output = NULL;

        return php_stream_alloc(&php_stream_brotli_write_ops, self, NULL, mode);

    } else {
        self->cctx = NULL;
        if (php_brotli_decoder_create(&self->dctx) != SUCCESS) {
            php_error_docref(NULL, E_WARNING,
                             "brotli: decompression context failed");
            php_stream_close(self->stream);
            efree(self);
            return NULL;
        }

        self->result = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;

        self->available_in = 0;
        self->next_in = NULL;
        self->available_out = 0;
        self->next_out = NULL;
        self->output = NULL;

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

    le_state = zend_register_list_destructors_ex(php_brotli_state_rsrc_dtor,
                                                 NULL, "brotli.state",
                                                 module_number);

    php_output_handler_alias_register(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER),
                                      php_brotli_output_handler_init);
    php_output_handler_conflict_register(ZEND_STRL(PHP_BROTLI_OUTPUT_HANDLER),
                                         php_brotli_output_conflict);

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
        php_brotli_output_compression_start();
    }
    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(brotli)
{
    if (BROTLI_G(handler_registered)) {
        php_brotli_cleanup_ob_handler_mess();
    }
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
    uint32_t version = BrotliEncoderVersion();
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d.%d.%d",
             version >> 24, (version >> 12) & 0xfff, version & 0xfff);
    php_info_print_table_row(2, "Library Version", buffer);
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

    long quality = BROTLI_DEFAULT_QUALITY;
    long mode =  BROTLI_MODE_GENERIC;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_STRING(in, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(quality)
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    size_t out_size = BrotliEncoderMaxCompressedSize(in_size);
    char *out = (char *)emalloc(out_size);

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
        php_error_docref(NULL, E_WARNING,
                         "Brotli compress failed");
        efree(out);
        RETURN_FALSE;
    }

    RETVAL_STRINGL(out, out_size);
    efree(out);
}

static ZEND_FUNCTION(brotli_compress_init)
{
    long quality = BROTLI_DEFAULT_QUALITY;
    long mode =  BROTLI_MODE_GENERIC;
    php_brotli_state_context *ctx;

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(quality)
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    ctx = php_brotli_state_init();

    if (php_brotli_encoder_create(&ctx->encoder,
                                  quality, 0, mode) != SUCCESS) {
        php_error_docref(NULL, E_WARNING,
                         "Brotli incremental compress init failed");
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(ctx, le_state));
}

static ZEND_FUNCTION(brotli_compress_add)
{
    zval *res;
    php_brotli_state_context *ctx;
    size_t buffer_size, buffer_used;
    zend_long mode = BROTLI_OPERATION_FLUSH;
    char *in_buf;
    size_t in_size;
    smart_string out = {0};

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_RESOURCE(res)
        Z_PARAM_STRING(in_buf, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    ctx = zend_fetch_resource(Z_RES_P(res), NULL, le_state);
    if (ctx == NULL || ctx->encoder == NULL) {
        php_error_docref(NULL, E_WARNING,
                         "Brotli incremental compress resource failed");
        RETURN_FALSE;
    }

    buffer_size = BrotliEncoderMaxCompressedSize(in_size);
    buffer_size = (buffer_size < 64) ? 64 : buffer_size;
    uint8_t *buffer = (uint8_t *)emalloc(buffer_size);

    const uint8_t *next_in = in_buf;
    size_t available_in = in_size;

    while (available_in || BrotliEncoderHasMoreOutput(ctx->encoder)) {
        size_t available_out = buffer_size;
        uint8_t *next_out = buffer;
        if (BrotliEncoderCompressStream(ctx->encoder,
                                        mode,
                                        &available_in,
                                        &next_in,
                                        &available_out,
                                        &next_out,
                                        0)) {
            buffer_used = (size_t)(next_out - buffer);
            if (buffer_used) {
                smart_string_appendl(&out, buffer, buffer_used);
            }
        } else {
            efree(buffer);
            smart_string_free(&out);
            php_error_docref(NULL, E_WARNING,
                             "Brotli incremental compress failed");
            RETURN_FALSE;
        }
    }

    if (mode == BROTLI_OPERATION_FINISH) {
        while (!BrotliEncoderIsFinished(ctx->encoder)) {
            size_t available_out = buffer_size;
            uint8_t *next_out = buffer;
            if (BrotliEncoderCompressStream(ctx->encoder,
                                            BROTLI_OPERATION_FINISH,
                                            &available_in,
                                            &next_in,
                                            &available_out,
                                            &next_out,
                                            0)) {
                buffer_used = (size_t)(next_out - buffer);
                if (buffer_used) {
                    smart_string_appendl(&out, buffer, buffer_used);
                }
            } else {
                efree(buffer);
                smart_string_free(&out);
                php_error_docref(NULL, E_WARNING,
                                 "Brotli incremental compress failed");
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
    long max_size = 0;
    char *in;
    size_t in_size;
    smart_string out = {0};

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(in, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(max_size)
    ZEND_PARSE_PARAMETERS_END();

    if (max_size && max_size < in_size) {
        in_size = max_size;
    }

    BrotliDecoderState *state = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    if (!state) {
        php_error_docref(NULL, E_WARNING,
                         "Invalid Brotli state");
        RETURN_FALSE;
    }

    size_t available_in = in_size;
    const uint8_t *next_in = (const uint8_t *)in;
    size_t buffer_size = PHP_BROTLI_BUFFER_SIZE;
    uint8_t *buffer = (uint8_t *)emalloc(buffer_size);

    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
    while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
        size_t available_out = buffer_size;
        uint8_t *next_out = buffer;
        result = BrotliDecoderDecompressStream(state, &available_in, &next_in,
                                               &available_out, &next_out,
                                               0);
        size_t used_out = buffer_size - available_out;
        if (used_out != 0) {
            smart_string_appendl(&out, buffer, used_out);
        }
    }

    BrotliDecoderDestroyInstance(state);
    efree(buffer);

    if (result != BROTLI_DECODER_RESULT_SUCCESS) {
        php_error_docref(NULL, E_WARNING,
                         "Brotli decompress failed");
        smart_string_free(&out);
        RETURN_FALSE;
    }

    RETVAL_STRINGL(out.c, out.len);
    smart_string_free(&out);
}

static ZEND_FUNCTION(brotli_uncompress_init)
{
    php_brotli_state_context *ctx;

    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_FALSE;
    }

    ctx = php_brotli_state_init();

    if (php_brotli_decoder_create(&ctx->decoder) != SUCCESS) {
        php_error_docref(NULL, E_WARNING,
                         "Brotli incremental uncompress init failed");
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(ctx, le_state));
}

static ZEND_FUNCTION(brotli_uncompress_add)
{
    zval *res;
    php_brotli_state_context *ctx;
    size_t buffer_size;
    zend_long mode = BROTLI_OPERATION_FLUSH;
    char *in_buf;
    size_t in_size;
    smart_string out = {0};

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_RESOURCE(res)
        Z_PARAM_STRING(in_buf, in_size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    ctx = zend_fetch_resource(Z_RES_P(res), NULL, le_state);
    if (ctx == NULL || ctx->decoder == NULL) {
        php_error_docref(NULL, E_WARNING,
                         "Brotli incremental uncompress resource failed");
        RETURN_FALSE;
    }

    if (in_size <= 0 && mode != BROTLI_OPERATION_FINISH) {
        RETURN_EMPTY_STRING();
    }

    buffer_size = PHP_BROTLI_BUFFER_SIZE;
    uint8_t *buffer = (uint8_t *)emalloc(buffer_size);

    const uint8_t *next_in = (const uint8_t *)in_buf;
    size_t available_in = in_size;

    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
    while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
        size_t available_out = buffer_size;
        uint8_t *next_out = buffer;
        result = BrotliDecoderDecompressStream(ctx->decoder,
                                               &available_in, &next_in,
                                               &available_out, &next_out,
                                               0);
        size_t buffer_used = buffer_size - available_out;
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
    int lgwin = BROTLI_DEFAULT_WINDOW, quality = BROTLI_DEFAULT_QUALITY;
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

    if (!BrotliEncoderCompress(quality, lgwin, mode,
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
                         "Error at offset %ld of %ld bytes",
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
