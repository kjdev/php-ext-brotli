--TEST--
use dictionary: streams
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
$dictionary = file_get_contents(dirname(__FILE__) . '/data.dict');
$file = dirname(__FILE__) . '/data_' . basename(__FILE__, ".php") . '.out';

echo "Compression\n";

$ctx = stream_context_create(
    array(
        "brotli" => array(
            "dict" => $dictionary,
        )
    )
);

var_dump(file_put_contents('compress.brotli://' . $file, $data, 0, $ctx) == strlen($data));
var_dump($size1 = filesize($file));
var_dump($size1 > 1 && $size1 <= strlen($data));

echo "Decompression\n";

$decomp = file_get_contents('compress.brotli://' . $file, false, $ctx);
var_dump($decomp == $data);

$comp = file_get_contents($file);
$decomp = brotli_uncompress($comp, $dictionary);
var_dump($decomp == $data);

@unlink($file);
?>
===DONE===
--EXPECTF--
Compression
bool(true)
int(%d)
bool(true)
Decompression
bool(true)
bool(true)
===DONE===
