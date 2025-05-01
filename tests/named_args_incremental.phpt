--TEST--
incremental function with use named arguments
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80000) die("skip requires PHP 8.0+");
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

function test($data, $level = 0, $mode = 0) {
    echo "level={$level}, mode={$mode}\n";

    $modeTypes = [
        'BROTLI_PROCESS' => BROTLI_PROCESS,
        'BROTLI_FLUSH' => BROTLI_FLUSH,
    ];

    foreach ($modeTypes as $modeTypeKey => $modeType) {
        $compressed = '';

        $context = brotli_compress_init(
            level: $level,
            mode: $mode,
        );
        if ($context === false) {
            echo "ERROR\n";
            return;
        }
        foreach (str_split($data, 6) as $var) {
            $compressed .= brotli_compress_add(
                context: $context,
                data: $var,
                mode: $modeType,
            );
        }
        $compressed .= brotli_compress_add(
            context: $context,
            data: '',
            mode: BROTLI_FINISH,
        );

        if ($data === brotli_uncompress(data: $compressed)) {
            echo "OK\n";
        } else {
            echo "ERROR: {$modeTypeKey}\n";
        }

        $out = '';
        $context = brotli_uncompress_init();
        foreach (str_split($compressed, 6) as $var) {
            $out .= brotli_uncompress_add(
                context: $context,
                data: $var,
                mode: $modeType,
            );
        }
        $out .= brotli_uncompress_add(
            context: $context,
            data: '',
            mode: BROTLI_FINISH,
        );

        if ($data === $out) {
            echo "Ok\n";
        } else {
            echo "Error: {$modeTypeKey}\n";
        }
    }
}

foreach ([0, 9, 11, 20, -1] as $level) {
    test($data, $level, 0);
}
foreach ([0, 1, 2, 3, -1] as $mode) {
    test($data, 0, $mode);
}
?>
===DONE===
--EXPECTF--
level=0, mode=0
OK
Ok
OK
Ok
level=9, mode=0
OK
Ok
OK
Ok
level=11, mode=0
OK
Ok
OK
Ok
level=20, mode=0

Warning: brotli_compress_init(): failed to compression level (20): must be within 0..11 in %s on line %d
ERROR
level=-1, mode=0

Warning: brotli_compress_init(): failed to compression level (-1): must be within 0..11 in %s on line %d
ERROR
level=0, mode=0
OK
Ok
OK
Ok
level=0, mode=1
OK
Ok
OK
Ok
level=0, mode=2
OK
Ok
OK
Ok
level=0, mode=3

Warning: brotli_compress_init(): failed to compression mode (3): must be BROTLI_GENERIC(0)|BROTLI_TEXT(1)|BROTLI_FONT(2) in %s on line %d
ERROR
level=0, mode=-1

Warning: brotli_compress_init(): failed to compression mode (-1): must be BROTLI_GENERIC(0)|BROTLI_TEXT(1)|BROTLI_FONT(2) in %s on line %d
ERROR
===DONE===
