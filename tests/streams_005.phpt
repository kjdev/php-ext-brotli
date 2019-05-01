--TEST--
compress.brotli read online stream
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '5.4', '<')) die('skip PHP is too old');
if (getenv("SKIP_ONLINE_TESTS")) die('skip online test');
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
