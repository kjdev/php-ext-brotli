--TEST--
brotli.output_compression_exclude_types match with charset parameter
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
?>
--GET--
a=b
--INI--
brotli.output_compression=1
brotli.output_compression_exclude_types=text/html
--ENV--
HTTP_ACCEPT_ENCODING=br
--FILE--
<?php
header("Content-Type: text/html; charset=UTF-8");
echo "hi\n";
?>
--EXPECT--
hi
