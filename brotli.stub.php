<?php

namespace {

  function brotli_compress(string $data, int $quality = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC): string|false {}

  function brotli_uncompress(string $data, int $length = 0): string|false {}

  /**
   * @return resource|false
   */
  function brotli_compress_init(int $quality = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC) {}

  /**
   * @param resource $context
   */
  function brotli_compress_add($context, string $data, int $mode = BROTLI_PROCESS): string|false {}

  /**
   * @return resource|false
   */
  function brotli_uncompress_init() {}

  /**
   * @param resource $context
   */
  function brotli_uncompress_add($context, string $data, int $mode = BROTLI_PROCESS): string|false {}
}

namespace Brotli {

  function compress(string $data, int $quality = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC): string|false {}

  function ncompress(string $data, int $length = 0): string|false {}

  /**
   * @return resource|false
   */
  function compress_init(int $quality = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC) {}

  /**
   * @param resource $context
   */
  function compress_add($context, string $data, int $mode = \BROTLI_PROCESS): string|false {}

  /**
   * @return resource|false
   */
  function uncompress_init() {}

  /**
   * @param resource $context
   */
  function uncompress_add($context, string $data, int $mode = \BROTLI_PROCESS): string|false {}
}

#if defined(PHP_WIN32) || defined(HAVE_DNS_SEARCH_FUNC)
#endif
