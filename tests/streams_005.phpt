--TEST--
compress.brotli read online stream
--SKIPIF--
<?php
if (getenv("SKIP_ONLINE_TESTS")) die('skip online test');
if (!extension_loaded('openssl')) die('skip reqiures ext-openssl for https stream');
?>
--INI--
allow_url_fopen=1
--FILE--
<?php
readfile("compress.brotli://https://github.com/google/brotli/raw/master/tests/testdata/x.compressed");
?>

===DONE===
--EXPECT--
X
===DONE===
