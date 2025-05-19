<?php

namespace {

  /**
   * @var int
   * @cvalue BROTLI_MODE_GENERIC
   */
  const BROTLI_GENERIC = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_MODE_TEXT
   */
  const BROTLI_TEXT = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_MODE_FONT
   */
  const BROTLI_FONT = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_MIN_QUALITY
   */
  const BROTLI_COMPRESS_LEVEL_MIN = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_MAX_QUALITY
   */
  const BROTLI_COMPRESS_LEVEL_MAX = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_DEFAULT_QUALITY
   */
  const BROTLI_COMPRESS_LEVEL_DEFAULT = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_OPERATION_PROCESS
   */
  const BROTLI_PROCESS = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_OPERATION_FLUSH
   */
  const BROTLI_FLUSH = UNKNOWN;

  /**
   * @var int
   * @cvalue BROTLI_OPERATION_FINISH
   */
  const BROTLI_FINISH = UNKNOWN;

  /**
   * @var bool
   * @cvalue USE_BROTLI_DICTIONARY
   * @note enabled when brotli library version is 1.1.0 or higher
   */
  const BROTLI_DICTIONARY_SUPPORT = UNKNOWN;

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function brotli_compress(string $data, int $level = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC, ?string $dict = null): string|false {}

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function brotli_uncompress(string $data, ?string $dict = null): string|false {}

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function brotli_compress_init(int $level = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC, ?string $dict = null): Brotli\Compress\Context|false {}

  function brotli_compress_add(Brotli\Compress\Context $context, string $data, int $mode = BROTLI_FLUSH): string|false {}

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function brotli_uncompress_init(?string $dict = null): Brotli\UnCompress\Context|false {}

  function brotli_uncompress_add(Brotli\UnCompress\Context $context, string $data, int $mode = BROTLI_FLUSH): string|false {}
}

namespace Brotli {

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function compress(string $data, int $level = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC, ?string $dict = null): string|false {}

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function uncompress(string $data, ?string $dict = null): string|false {}

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function compress_init(int $level = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC, ?string $dict = null): Compress\Context|false {}

  function compress_add(Compress\Context $context, string $data, int $mode = \BROTLI_FLUSH): string|false {}

  /**
   * @note dict parameter can be used when BROTLI_DICTIONARY_SUPPORT is enabled
   */
  function uncompress_init(?string $dict = null): UnCompress\Context|false {}

  function uncompress_add(UnCompress\Context $context, string $data, int $mode = \BROTLI_FLUSH): string|false {}

}

namespace Brotli\Compress {

  final class Context
  {
  }

}

namespace Brotli\UnCompress {

  final class Context
  {
  }

}

#if defined(PHP_WIN32) || defined(HAVE_DNS_SEARCH_FUNC)
#endif
