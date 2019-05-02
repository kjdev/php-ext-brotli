--TEST--
alias (namespace) functionality
--SKIPIF--
<?php if (PHP_VERSION_ID < 50300) die("Skipped: PHP 5.3+ required."); ?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

echo "Compression\n";

$compressed = \Brotli\compress($data);
var_dump($data == brotli_uncompress($compressed));
var_dump($data == \Brotli\uncompress($compressed));

echo "Decompression\n";

$compressed = brotli_compress($data);
var_dump($data == brotli_uncompress($compressed));
var_dump($data == \Brotli\uncompress($compressed));
?>
===DONE===
--EXPECTF--
Compression
bool(true)
bool(true)
Decompression
bool(true)
bool(true)
===DONE===
