--TEST--
brotli.output_compression
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip');
?>
--INI--
brotli.output_compression=1
--ENV--
HTTP_ACCEPT_ENCODING=br
--FILE--
<?php
ini_set('brotli.output_compression', 0);
echo "hi\n";
?>
--EXPECT--
hi
