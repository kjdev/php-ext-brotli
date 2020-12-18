--TEST--
compress.brotli read online stream denied
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '5.4', '<')) die('skip PHP is too old');
if (getenv("SKIP_ONLINE_TESTS")) die('skip online test');
if (!extension_loaded('openssl')) die('skip reqiures ext-openssl for https stream');
?>
--INI--
allow_url_fopen=0
--FILE--
<?php
readfile("compress.brotli://https://github.com/google/brotli/raw/master/tests/testdata/x.compressed");
?>

===DONE===
--EXPECTF--
Warning: readfile(): https:// wrapper is disabled in the server configuration by allow_url_fopen=0 in %s

Warning: readfile(https://github.com/google/brotli/raw/master/tests/testdata/x.compressed): %sailed to open stream: no suitable wrapper could be found in %s

Warning: readfile(compress.brotli://https://github.com/google/brotli/raw/master/tests/testdata/x.compressed): %sailed to open stream: operation failed in %s

===DONE===
