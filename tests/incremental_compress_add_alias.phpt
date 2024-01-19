--TEST--
Test incremental Brotli\compress_add() alias (namespace) functionality
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

  $resource = Brotli\compress_init();
  foreach (range('a', 'z') as $c) {
    $uncompressed .= $c;
    $compressed .= Brotli\compress_add($resource, $c, $modeType);
  }
  $compressed .= Brotli\compress_add($resource, '', BROTLI_FINISH);

  if ($uncompressed !== brotli_uncompress($compressed)) {
    echo "Error: Brotli\\compress_add | {$modeTypeKey}\n";
  }
}
?>
===DONE===
--EXPECT--
===DONE===
