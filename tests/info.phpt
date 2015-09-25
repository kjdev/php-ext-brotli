--TEST--
Test phpinfo() displays brotli info
--SKIPIF--
--FILE--
<?php
if (!extension_loaded('brotli')) {
    dl('brotli.' . PHP_SHLIB_SUFFIX);
}

phpinfo();
--EXPECTF--
%a
brotli

Brotli support => enabled
Extension Version => %d.%d.%d
Library Version => %d.%d.%d
%a
