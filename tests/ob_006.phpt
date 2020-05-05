--TEST--
brotli.output_compression=1 with client not accepting compression
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip');
if (version_compare(PHP_VERSION, '5.4.0', '<')) die('skip need version');
?>
--INI--
brotli.output_compression=1
display_startup_errors=1
--FILE--
===DONE===
--EXPECT--
===DONE===
