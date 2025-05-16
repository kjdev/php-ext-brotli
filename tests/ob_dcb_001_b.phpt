--TEST--
output handler: dcb
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
if (substr(PHP_OS, 0, 3) !== 'WIN') die('skip need Windows');
?>
--GET--
ob=dictionary
--ENV--
HTTP_ACCEPT_ENCODING=dcb
HTTP_AVAILABLE_DICTIONARY=:hO22zM5/9TAGlAldeA8WvH1bvEv1LqjRjzzw3Zf9p4M=:
--FILE--
<?php

ini_set('brotli.output_compression', 1);
ini_set('brotli.output_compression_dict', dirname(__FILE__) . '/data.dict');

include(dirname(__FILE__) . '/data.inc');
echo "{$data}";
?>
--EXPECTF--
%a
--EXPECTHEADERS--
Content-Encoding: dcb
Vary: Accept-Encoding, Available-Dictionary
