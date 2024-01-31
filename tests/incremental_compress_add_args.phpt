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

  $uncompressed = $compressed = '';

  $resource = brotli_compress_init($quality, $mode);
  foreach (range('a', 'z') as $c) {
    $uncompressed .= $c;
    $compressed .= brotli_compress_add($resource, $c, BROTLI_PROCESS);
  }
  $compressed .= brotli_compress_add($resource, '', BROTLI_FINISH);

  if ($uncompressed === brotli_uncompress($compressed)) {
    echo "OK\n";
  } else {
    echo "Error: brotli_compress_add\n";
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
quality=9, mode=0
OK
quality=11, mode=0
OK
quality=20, mode=0

Warning: brotli_compress_init(): brotli: compression level (20) must be within 0..11 in %s on line %d
OK
quality=-1, mode=0

Warning: brotli_compress_init(): brotli: compression level (-1) must be within 0..11 in %s on line %d
OK
quality=0, mode=0
OK
quality=0, mode=1
OK
quality=0, mode=2
OK
quality=0, mode=3

Warning: brotli_compress_init(): brotli: compression mode (3) must be 0, 1, 2 in %s on line %d
OK
quality=0, mode=-1

Warning: brotli_compress_init(): brotli: compression mode (-1) must be 0, 1, 2 in %s on line %d
OK
===DONE===
