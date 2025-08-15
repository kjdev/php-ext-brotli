--TEST--
output handler: br,dcb: invalid output_compression_dict
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
?>
--GET--
ob=dictionary
--ENV--
HTTP_ACCEPT_ENCODING=br,dcb
HTTP_AVAILABLE_DICTIONARY=:4SycXHyLhvKPL/nNaYXl60myMwris2L4X1O+q5tW1XE=:
--FILE--
<?php

ini_set('brotli.output_compression', 1);
ini_set('brotli.output_compression_dict', '');

include(dirname(__FILE__) . '/data.inc');
echo "{$data}";
?>
--EXPECT_EXTERNAL--
files/ob_data.br
--EXPECTHEADERS--
Content-Encoding: br
Vary: Accept-Encoding
