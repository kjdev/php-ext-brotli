--TEST--
Test incremental brotli_compress_add() functionality
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (PHP_VERSION_ID < 70000) die('skip need version: 7.0+');
?>
--FILE--
<?php
$modeTypes = [
  'BROTLI_PROCESS' => BROTLI_PROCESS,
  'BROTLI_FLUSH' => BROTLI_FLUSH,
];

foreach ($modeTypes as $modeTypeKey => $modeType) {
  $uncompressed = $compressed = '';

  $resource = brotli_compress_init();

  foreach (range('a', 'z') as $c) {
    $uncompressed .= $c;
    $compressed .= brotli_compress_add($resource, $c, $modeType);
  }
  $compressed .= brotli_compress_add($resource, '', BROTLI_FINISH);

  if ($uncompressed !== brotli_uncompress($compressed)) {
    echo "Error: brotli_compress_add | {$modeTypeKey}\n";
  }
}
?>
===DONE===
--EXPECT--
===DONE===
