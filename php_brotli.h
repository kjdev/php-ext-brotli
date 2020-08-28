#ifndef PHP_BROTLI_H
#define PHP_BROTLI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Don't forget to check BROTLI_LIB_VERSION in config.m4/w32 */
#define BROTLI_EXT_VERSION "0.11.1"
#define BROTLI_NS "Brotli"

/* brotli */
#include "brotli/encode.h"
#include "brotli/decode.h"

extern zend_module_entry brotli_module_entry;
#define phpext_brotli_ptr &brotli_module_entry

/* support php-5.2m backport from php-5.3 */
#ifndef ZEND_FE_END
#define ZEND_FE_END            { NULL, NULL, NULL, 0, 0 }
#endif

#ifdef PHP_WIN32
#    define PHP_BROTLI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#    define PHP_BROTLI_API __attribute__ ((visibility("default")))
#else
#    define PHP_BROTLI_API
#endif

#ifdef ZTS
#    include "TSRM.h"
#endif

#if PHP_MAJOR_VERSION < 7
typedef long zend_long;
#endif

typedef struct _php_brotli_state_context {
    BrotliEncoderState *encoder;
    BrotliDecoderState *decoder;
} php_brotli_state_context;

typedef struct _php_brotli_context {
    php_brotli_state_context state;
    size_t available_in;
    const uint8_t *next_in;
    size_t available_out;
    uint8_t *next_out;
    uint8_t *output;
} php_brotli_context;

ZEND_BEGIN_MODULE_GLOBALS(brotli)
#if PHP_VERSION_ID > 50400 // Output Handler: 5.4+
  zend_long output_compression;
  zend_long output_compression_level;
  zend_bool handler_registered;
  int compression_coding;
  php_brotli_context *ob_handler;
#endif
ZEND_END_MODULE_GLOBALS(brotli)

#ifdef ZTS
#    define BROTLI_G(v) TSRMG(brotli_globals_id, zend_brotli_globals *, v)
#else
#    define BROTLI_G(v) (brotli_globals.v)
#endif

#ifdef __cplusplus
}
#endif

#endif  /* PHP_BROTLI_H */
