--TEST--
compress.brotli streams and compatibility
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

$file = dirname(__FILE__) . '/data.out';

echo "Stream compression + brotli_uncompress\n";

var_dump(file_put_contents('compress.brotli://' . $file, $data) == strlen($data));

$actual = brotli_uncompress(file_get_contents($file));
var_dump($actual === $data);

@unlink($file);

echo "brotli_compress + Stream decompression\n";

var_dump(file_put_contents($file, brotli_compress($data)));
$decomp = file_get_contents('compress.brotli://' . $file);
var_dump($actual == $data);

@unlink($file);
?>
===DONE===
--EXPECTF--
Stream compression + brotli_uncompress
bool(true)
bool(true)
brotli_compress + Stream decompression
int(%d)
bool(true)
===DONE===
