--TEST--
ob_brotli_handler always conflicts with brotli.output_compression
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
?>
--INI--
output_handler=ob_brotli_handler
brotli.output_compression=0
--ENV--
HTTP_ACCEPT_ENCODING=br
--FILE--
<?php
echo "hi";
?>
--EXPECT_EXTERNAL--
files/ob_008.br
--EXPECTHEADERS--
Content-Encoding: br
Vary: Accept-Encoding
