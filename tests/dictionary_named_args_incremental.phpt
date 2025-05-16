--TEST--
use dictionary: incremental with named arguments
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80000) die("skip requires PHP 8.0+");
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
$dictionary = file_get_contents(dirname(__FILE__) . '/data.dict');

$compressed = '';

$context = brotli_compress_init(dict: $dictionary);
if ($context === false) {
    echo "ERROR\n";
    die;
}
foreach (str_split($data, 10) as $var) {
    $compressed .= brotli_compress_add(
        context: $context,
        data: $var,
    );
}
$compressed .= brotli_compress_add(
    context: $context,
    data: '',
    mode: BROTLI_FINISH,
);

if ($data === brotli_uncompress(data: $compressed, dict: $dictionary)) {
    echo "OK\n";
} else {
    echo "ERROR\n";
}

$out = '';
$context = brotli_uncompress_init(dict: $dictionary);
foreach (str_split($compressed, 10) as $var) {
    $out .= brotli_uncompress_add(
        context: $context,
        data: $var,
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
    echo "Error\n";
}
?>
===DONE===
--EXPECTF--
OK
Ok
===DONE===
