#ifndef PHP_BROTLI_H
#define PHP_BROTLI_H

#ifdef __cplusplus
extern "C" {
#endif

#define BROTLI_EXT_VERSION "0.5.2"

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

#ifdef ZTS
#    define BROTLI_G(v) TSRMG(brotli_globals_id, zend_brotli_globals *, v)
#else
#    define BROTLI_G(v) (brotli_globals.v)
#endif

#ifdef __cplusplus
}
#endif

#endif  /* PHP_BROTLI_H */
