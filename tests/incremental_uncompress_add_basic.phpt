--TEST--
Test incremental brotli_uncompress_add() functionality
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
  $uncompressed = '';
  for ($i = 0; $i < (32768 * 2); $i++) {
    $uncompressed .= chr(rand(48, 125));
  }

  $compressed = brotli_compress($uncompressed);
  $compressedLen = strlen($compressed);

  $out = '';

  $resource = brotli_uncompress_init();
  for ($i = 0; $i < $compressedLen; $i++) {
    $out .= brotli_uncompress_add($resource, $compressed[$i], $modeType);
  }
  $out .= brotli_uncompress_add($resource, '', BROTLI_FINISH);

  if ($out !== $uncompressed) {
    echo "Error: brotli_uncompress_add | {$modeTypeKey}\n";
  }
}
?>
===DONE===
--EXPECT--
===DONE===
