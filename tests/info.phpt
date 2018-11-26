--TEST--
Test phpinfo() displays brotli info
--SKIPIF--
<?php if (!extension_loaded('brotli')) die('skip'); ?>
--FILE--
<?php
phpinfo();
--EXPECTF--
%a
brotli

Brotli support => enabled
Extension Version => %d.%d.%d
Library Version => %d.%d.%d
%a
