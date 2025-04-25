--TEST--
use dictionary: arguments
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
$dictionary = file_get_contents(dirname(__FILE__) . '/data.dict');

function check($data, $dictionary, $level, $mode)
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

    $output = brotli_compress($data, $level, $mode, $dictionary);
    echo $level, ' -- ', $modeName, ' -- ',
        ($dictionary === null ? 'null' : 'dict'), ' -- ',
        var_export(
            (($output === false) ? false
             : brotli_uncompress($output, 0, $dictionary) === $data),
            true
        ), PHP_EOL;
}

echo "*** Compression Level ***", PHP_EOL;
$mode = BROTLI_GENERIC;
for (
    $level = BROTLI_COMPRESS_LEVEL_MIN;
    $level <= BROTLI_COMPRESS_LEVEL_MAX;
    $level++
) {
    check($data, $dictionary, $level, $mode);
}

echo "*** Compression Mode ***", PHP_EOL;
$level = BROTLI_COMPRESS_LEVEL_DEFAULT;
check($data, $dictionary, $level, BROTLI_GENERIC);
check($data, $dictionary, $level, BROTLI_TEXT);
check($data, $dictionary, $level, BROTLI_FONT);

$mode = BROTLI_GENERIC;

echo "*** Compression Dictionary ***", PHP_EOL;
check($data, null, $level, $mode);
?>
===DONE===
--EXPECTF--
*** Compression Level ***
0 -- generic -- dict -- true
1 -- generic -- dict -- true
2 -- generic -- dict -- true
3 -- generic -- dict -- true
4 -- generic -- dict -- true
5 -- generic -- dict -- true
6 -- generic -- dict -- true
7 -- generic -- dict -- true
8 -- generic -- dict -- true
9 -- generic -- dict -- true
10 -- generic -- dict -- true
11 -- generic -- dict -- true
*** Compression Mode ***
11 -- generic -- dict -- true
11 -- text -- dict -- true
11 -- font -- dict -- true
*** Compression Dictionary ***
11 -- generic -- null -- true
===DONE===
