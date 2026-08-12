--TEST--
built-in output compression exclude list: wildcard match (no ini set)
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
header("Content-Type: audio/mpeg");
echo "hi\n";
?>
--EXPECT--
hi
