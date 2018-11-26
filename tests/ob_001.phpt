--TEST--
brotli.output_compression
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
if (version_compare(PHP_VERSION, '5.4.0', '<') die('skip need version');
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
