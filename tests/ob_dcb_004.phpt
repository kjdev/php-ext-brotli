--TEST--
output handler: br,dcb: invalid available-dictionary
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
?>
--GET--
ob=dictionary
--ENV--
HTTP_ACCEPT_ENCODING=br,dcb
HTTP_AVAILABLE_DICTIONARY=:test:
--FILE--
<?php

ini_set('brotli.output_compression', 1);
ini_set('brotli.output_compression_dict', dirname(__FILE__) . '/data.dict');

include(dirname(__FILE__) . '/data.inc');
echo "{$data}";
?>
--EXPECT_EXTERNAL--
files/ob_data.br
--EXPECTHEADERS--
Content-Encoding: br
Vary: Accept-Encoding
