#ifndef BROTLI_MIMETYPE_EXCLUDE

/**
 * Image types listed separately, because svg is quite common
 * and compresses well.
 *
 * Less common types have both, x-prefixed and non-prefixed
 * versions because they are somewhat inconsistent with each
 * other and perhaps more likely to be misconfigured due to
 * those inconsistencies.
 **/
#define BROTLI_MIMETYPE_EXCLUDE "\
video/*, \
audio/*, \
image/png, \
image/gif, \
image/jpeg, \
image/jxl, \
image/jp2, \
image/jpm, \
image/webp, \
image/avif, \
image/x-icon, image/vnd.microsoft.icon, \
font/woff, application/font-woff, application/x-font-woff, \
font/woff2, \
application/pdf, application/x-pdf, \
application/zip, application/x-zip, application/zip-compressed, application/x-zip-compressed, \
application/7z-compressed, application/x-7z-compressed, \
application/vnd.rar, application/x-vnd.rar, \
application/gzip, application/x-gzip, \
application/zstd, application/x-zstd, \
application/br, application/x-br, \
application/lz4, application/x-lz4, \
application/bzip2, application/x-bzip2\
"

#endif
