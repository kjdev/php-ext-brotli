--TEST--
brotli.output_compression_exclude_types later negation token wins over earlier positive match
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
?>
--GET--
a=b
--INI--
brotli.output_compression=1
brotli.output_compression_exclude_types="image/*,!image/png"
--ENV--
HTTP_ACCEPT_ENCODING=br
--FILE--
<?php
header("Content-Type: image/png");
echo "hi\n";
?>
--EXPECT_EXTERNAL--
files/ob_hi.br
--EXPECTHEADERS--
Content-Encoding: br
Vary: Accept-Encoding
