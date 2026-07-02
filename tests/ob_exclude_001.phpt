--TEST--
brotli.output_compression_exclude_types exact match
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
?>
--GET--
a=b
--INI--
brotli.output_compression=1
brotli.output_compression_exclude_types=application/pdf
--ENV--
HTTP_ACCEPT_ENCODING=br
--FILE--
<?php
header("Content-Type: application/pdf");
echo "hi\n";
?>
--EXPECT--
hi
