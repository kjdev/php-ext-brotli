--TEST--
brotli.output_compression Overwrites Vary Header
--INI--
brotli.output_compression=1
--ENV--
HTTP_ACCEPT_ENCODING=br
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
?>
--FILE--
<?php
header('Vary: Cookie');
echo 'foo';
?>
--EXPECTF--
%a
--EXPECTHEADERS--
Vary: Cookie
Content-Encoding: br
Vary: Accept-Encoding
