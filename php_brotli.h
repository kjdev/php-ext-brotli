#ifndef PHP_BROTLI_H
#define PHP_BROTLI_H

#ifdef __cplusplus
extern "C" {
#endif

#define BROTLI_EXT_VERSION "0.18.2"
#define BROTLI_NS "Brotli"

/* brotli */
#include "brotli/encode.h"
#include "brotli/decode.h"

extern zend_module_entry brotli_module_entry;
#define phpext_brotli_ptr &brotli_module_entry

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

typedef struct _php_brotli_context php_brotli_context;

ZEND_BEGIN_MODULE_GLOBALS(brotli)
  zend_long output_compression;
  zend_long output_compression_default;
  zend_long output_compression_level;
  char *output_compression_dict;
  zend_bool handler_registered;
  int compression_coding;
  php_brotli_context *ob_handler;
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
