--TEST--
Test incremental brotli_uncompress_add() functionality
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
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

$resource = brotli_uncompress_init();
for ($i = 0; $i < $compressedLen; $i++) {
  $out .= brotli_uncompress_add($resource, $compressed[$i], BROTLI_PROCESS);
}
$out .= brotli_uncompress_add($resource, '', BROTLI_FINISH);

if ($out !== $uncompressed) {
  echo "Error: brotli_uncompress_add\n";
}
?>
===DONE===
--EXPECT--
===DONE===
