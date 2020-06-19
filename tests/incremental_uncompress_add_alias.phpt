--TEST--
Test incremental Brotli\uncompress_add() alias (namespace) functionality
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (PHP_VERSION_ID < 70000) die('skip need version: 7.0+');
?>
--FILE--
<?php
$uncompressed = '';
for ($i = 0; $i < (32768 * 2); $i++) {
  $uncompressed .= chr(rand(48, 125));
}

$compressed = brotli_compress($uncompressed);
$compressedLen = strlen($compressed);

$out = '';

$resource = Brotli\uncompress_init();
for ($i = 0; $i < $compressedLen; $i++) {
  $out .= Brotli\uncompress_add($resource, $compressed[$i], BROTLI_PROCESS);
}
$out .= Brotli\uncompress_add($resource, '', BROTLI_FINISH);

if ($out !== $uncompressed) {
  echo "Error: Brotli\\uncompress_add\n";
}
?>
===DONE===
--EXPECT--
===DONE===
