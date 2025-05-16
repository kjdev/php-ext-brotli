--TEST--
output handler: dcb: invalid available-dictionary
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
if (false === stristr(PHP_SAPI, 'cgi')) die('skip need sapi/cgi');
?>
--GET--
ob=dictionary
--ENV--
HTTP_ACCEPT_ENCODING=dcb
HTTP_AVAILABLE_DICTIONARY=:test:
--FILE--
<?php

ini_set('brotli.output_compression', 1);
ini_set('brotli.output_compression_dict', dirname(__FILE__) . '/data.dict');

include(dirname(__FILE__) . '/data.inc');
echo "{$data}";
?>
--EXPECTF--
%a
Notice: %s: brotli: invalid available-dictionary: request(:test:) != actual(4SycXHyLhvKPL/nNaYXl60myMwris2L4X1O+q5tW1XE=) in Unknown on line 0
