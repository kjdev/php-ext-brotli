--TEST--
brotli_compress() functionality with arguments
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

function test($data, $level = 0, $mode = 0) {
    echo "level={$level}, mode={$mode}\n";

    $compressed = brotli_compress($data, $level, $mode);
    if ($compressed === false) {
        echo "ERROR\n";
        return;
    }

    if ($data === brotli_uncompress($compressed)) {
        echo "OK\n";
    } else {
        echo "Error: brotli_compress\n";
    }
}

foreach ([0, 9, 11, 20, -1] as $level) {
    test($data, $level, BROTLI_GENERIC);
}
foreach ([0, 1, 2, 3, -1] as $mode) {
    test($data, BROTLI_COMPRESS_LEVEL_DEFAULT, $mode);
}
?>
===DONE===
--EXPECTF--
level=0, mode=0
OK
level=9, mode=0
OK
level=11, mode=0
OK
level=20, mode=0

Warning: brotli_compress(): failed to compression level (20): must be within 0..11 in %s on line %d
ERROR
level=-1, mode=0

Warning: brotli_compress(): failed to compression level (-1): must be within 0..11 in %s on line %d
ERROR
level=11, mode=0
OK
level=11, mode=1
OK
level=11, mode=2
OK
level=11, mode=3

Warning: brotli_compress(): failed to compression mode (3): must be BROTLI_GENERIC(0)|BROTLI_TEXT(1)|BROTLI_FONT(2) in %s on line %d
ERROR
level=11, mode=-1

Warning: brotli_compress(): failed to compression mode (-1): must be BROTLI_GENERIC(0)|BROTLI_TEXT(1)|BROTLI_FONT(2) in %s on line %d
ERROR
===DONE===
