--TEST--
compress.brotli streams basic
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

$file = dirname(__FILE__) . '/data.out';

echo "Compression with defaul level\n";

var_dump(file_put_contents('compress.brotli://' . $file, $data) == strlen($data));
var_dump($size1 = filesize($file));
var_dump($size1 > 1 && $size1 < strlen($data));

echo "Compression with specfic level\n";

$ctx = stream_context_create(
  array(
    "brotli" => array(
      "level" => 6,
    )
  )
);

var_dump(file_put_contents('compress.brotli://' . $file, $data, 0, $ctx) == strlen($data));
var_dump($size2 = filesize($file));
var_dump($size2 > 1 && $size2 <= $size1);

echo "Decompression\n";

$decomp = file_get_contents('compress.brotli://' . $file);
var_dump($decomp == $data);

@unlink($file);
?>
===DONE===
--EXPECTF--
Compression with defaul level
bool(true)
int(%d)
bool(true)
Compression with specfic level
bool(true)
int(%d)
bool(true)
Decompression
bool(true)
===DONE===
