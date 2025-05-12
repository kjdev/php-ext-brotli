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


  function brotli_compress(string $data, int $level = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC): string|false {}

  function brotli_uncompress(string $data, int $length = 0): string|false {}

  function brotli_compress_init(int $level = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC): Brotli\Compress\Context|false {}

  function brotli_compress_add(Brotli\Compress\Context $context, string $data, int $mode = BROTLI_FLUSH): string|false {}

  function brotli_uncompress_init(): Brotli\UnCompress\Context|false {}

  function brotli_uncompress_add(Brotli\UnCompress\Context $context, string $data, int $mode = BROTLI_FLUSH): string|false {}
}

namespace Brotli {

  function compress(string $data, int $level = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC): string|false {}

  function uncompress(string $data, int $length = 0): string|false {}

  function compress_init(int $level = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC): Compress\Context|false {}

  function compress_add(Compress\Context $context, string $data, int $mode = \BROTLI_FLUSH): string|false {}

  function uncompress_init(): UnCompress\Context|false {}

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
