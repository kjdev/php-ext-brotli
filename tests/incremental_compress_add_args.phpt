--TEST--
Test incremental brotli_compress_add() functionality with arguments
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
?>
--FILE--
<?php
function test($quality = 0, $mode = 0) {
  echo "quality={$quality}, mode={$mode}\n";

  $modeTypes = [
    'BROTLI_PROCESS' => BROTLI_PROCESS,
    'BROTLI_FLUSH' => BROTLI_FLUSH,
  ];

  foreach ($modeTypes as $modeTypeKey => $modeType) {

    $uncompressed = $compressed = '';

    $resource = brotli_compress_init($quality, $mode);
    if ($resource === false) {
      echo "ERROR\n";
      return;
    }
    foreach (range('a', 'z') as $c) {
      $uncompressed .= $c;
      $compressed .= brotli_compress_add($resource, $c, $modeType);
    }
    $compressed .= brotli_compress_add($resource, '', BROTLI_FINISH);

    if ($uncompressed === brotli_uncompress($compressed)) {
      echo "OK\n";
    } else {
      echo "Error: brotli_compress_add | {$modeTypeKey}\n";
    }
  }
}

foreach ([0, 9, 11, 20, -1] as $quality) {
  test($quality, 0);
}
foreach ([0, 1, 2, 3, -1] as $mode) {
  test(0, $mode);
}
?>
===DONE===
--EXPECTF--
quality=0, mode=0
OK
OK
quality=9, mode=0
OK
OK
quality=11, mode=0
OK
OK
quality=20, mode=0

Warning: brotli_compress_init(): quality (20) must be within 0..11 in %s on line %d
ERROR
quality=-1, mode=0

Warning: brotli_compress_init(): quality (-1) must be within 0..11 in %s on line %d
ERROR
quality=0, mode=0
OK
OK
quality=0, mode=1
OK
OK
quality=0, mode=2
OK
OK
quality=0, mode=3

Warning: brotli_compress_init(): mode (3) must be BROTLI_GENERIC(0)|BROTLI_TEXT(1)|BROTLI_FONT(2) in %s on line %d
ERROR
quality=0, mode=-1

Warning: brotli_compress_init(): mode (-1) must be BROTLI_GENERIC(0)|BROTLI_TEXT(1)|BROTLI_FONT(2) in %s on line %d
ERROR
===DONE===
