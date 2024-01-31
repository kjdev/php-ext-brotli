--TEST--
brotli.output_compression
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
?>
--GET--
a=b
--INI--
brotli.output_compression=1
--ENV--
HTTP_ACCEPT_ENCODING=br
--FILE--
<?php
echo "hi\n";
?>
--EXPECTF--
€hi

