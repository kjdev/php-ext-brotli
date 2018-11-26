--TEST--
ob_brotli_handler
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
if (version_compare(PHP_VERSION, '5.4.0', '<') die('skip need version');
?>
--INI--
brotli.output_compression=0
--ENV--
HTTP_ACCEPT_ENCODING=br
--POST--
dummy=42
--FILE--
<?php
ob_start('ob_brotli_handler');
ini_set('brotli.output_compression', 0);
echo "hi\n";
?>
--EXPECT--
hi
--EXPECTHEADERS--
