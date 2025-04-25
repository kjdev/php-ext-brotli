--TEST--
dictionary functionality
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
$dictionary = file_get_contents(dirname(__FILE__) . '/data.dict');

$quality = BROTLI_COMPRESS_LEVEL_DEFAULT;
$mode = BROTLI_GENERIC;

echo "*** Dictionary Compression ***", PHP_EOL;

$out1 = brotli_compress($data, $quality, $mode, $dictionary);
$out2 = \Brotli\compress($data, $quality, $mode, $dictionary);

var_dump(brotli_uncompress($out1, 0, $dictionary) === $data);
var_dump(brotli_uncompress($out2, 0, $dictionary) === $data);
var_dump(\Brotli\uncompress($out1, 0, $dictionary) === $data);
var_dump(\Brotli\uncompress($out2, 0, $dictionary) === $data);
?>
===Done===
--EXPECTF--
*** Dictionary Compression ***
bool(true)
bool(true)
bool(true)
bool(true)
===Done===
