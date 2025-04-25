--TEST--
dictionary functionality: compress level
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
$dictionary = file_get_contents(dirname(__FILE__) . '/data.dict');

function check($data, $dictionary, $quality, $mode)
{
    switch ($mode) {
        case BROTLI_GENERIC:
            $modeName = 'generic';
            break;
        case BROTLI_TEXT:
            $modeName = 'text';
            break;
        case BROTLI_FONT:
            $modeName = 'font';
            break;
        default:
            $modeName = '';
    }

    $output = brotli_compress($data, $quality, $mode, $dictionary);
    echo $quality, ' -- ', $modeName, ' -- ',
        var_export(
            (($output === false) ? false
             : brotli_uncompress($output, 0, $dictionary) === $data),
            true
        ), PHP_EOL;
}

echo "*** Compression Level ***", PHP_EOL;
$mode = BROTLI_GENERIC;
for (
    $quality = BROTLI_COMPRESS_LEVEL_MIN;
    $quality <= BROTLI_COMPRESS_LEVEL_MAX;
    $quality++
) {
    check($data, $dictionary, $quality, $mode);
}

echo "*** Compression Mode ***", PHP_EOL;
$quality = BROTLI_COMPRESS_LEVEL_DEFAULT;
check($data, $dictionary, $quality, BROTLI_GENERIC);
check($data, $dictionary, $quality, BROTLI_TEXT);
check($data, $dictionary, $quality, BROTLI_FONT);

$mode = BROTLI_GENERIC;

echo "*** Invalid Compression Level ***", PHP_EOL;
check($data, $dictionary, -1, $mode);
check($data, $dictionary, 100, $mode);

echo "*** Invalid Compression Mode ***", PHP_EOL;
check($data, $dictionary, $quality, 10);
?>
===Done===
--EXPECTF--
*** Compression Level ***
0 -- generic -- true
1 -- generic -- true
2 -- generic -- true
3 -- generic -- true
4 -- generic -- true
5 -- generic -- true
6 -- generic -- true
7 -- generic -- true
8 -- generic -- true
9 -- generic -- true
10 -- generic -- true
11 -- generic -- true
*** Compression Mode ***
11 -- generic -- true
11 -- text -- true
11 -- font -- true
*** Invalid Compression Level ***

Warning: brotli_compress(): quality (-1) must be within 0..11 in %s on line %d
-1 -- generic -- false

Warning: brotli_compress(): quality (100) must be within 0..11 in %s on line %d
100 -- generic -- false
*** Invalid Compression Mode ***

Warning: brotli_compress(): mode (10) must be BROTLI_GENERIC(0)|BROTLI_TEXT(1)|BROTLI_FONT(2) in %s on line %d
11 --  -- false
===Done===
