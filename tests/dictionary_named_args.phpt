--TEST--
use dictionary: named arguments
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80000) die("skip requires PHP 8.0+");
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
$dictionary = file_get_contents(dirname(__FILE__) . '/data.dict');
$level = BROTLI_COMPRESS_LEVEL_MAX;
$mode = BROTLI_TEXT;

echo "** brotli_compress(dict:) **\n";
try {
    var_dump(gettype(brotli_compress(dict: $dictionary)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(data:, dict:) **\n";
try {
    var_dump(gettype(brotli_compress(data: $data, dict: $dictionary)));
    var_dump(brotli_uncompress(brotli_compress(data: $data, dict: $dictionary), dict: $dictionary) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(level:, dict:) **\n";
try {
    var_dump(gettype(brotli_compress(level: $level, dict: $dictionary)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(mode:, dict:) **\n";
try {
    var_dump(gettype(brotli_compress(mode: $mode, dict: $dictionary)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(data:, level:, dict:) **\n";
try {
    var_dump(gettype(brotli_compress(data: $data, level: $level, dict: $dictionary)));
    var_dump(brotli_uncompress(brotli_compress(data: $data, level: $level, dict: $dictionary), dict: $dictionary) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(data:, mode:, dict:) **\n";
try {
    var_dump(gettype(brotli_compress(data: $data, mode: $mode, dict: $dictionary)));
    var_dump(brotli_uncompress(brotli_compress(data: $data, mode: $mode, dict: $dictionary), dict: $dictionary) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(level:, mode:, dict:) **\n";
try {
    var_dump(gettype(brotli_compress(level: $level, mode: $mode, dict: $dictionary)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

$compressed = brotli_compress(data: $data, dict: $dictionary);

echo "** brotli_uncompress(dict:): false **\n";
try {
    var_dump(gettype(brotli_uncompress(dict: $dictionary)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_uncompress(data:, dict:) **\n";
try {
    var_dump(gettype(brotli_uncompress(data: $compressed, dict: $dictionary)));
    var_dump(brotli_uncompress(data: $compressed, dict: $dictionary) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_uncompress(length:, dict:) **\n";
try {
    var_dump(gettype(brotli_uncompress(length: strlen($compressed), dict: $dictionary)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_uncompress(data: length:, dict:) **\n";
try {
    var_dump(gettype(brotli_uncompress(data: $compressed, length: strlen($compressed), dict: $dictionary)));
    var_dump(brotli_uncompress(data: $compressed, length: strlen($compressed), dict: $dictionary) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}
?>
===DONE===
--EXPECTF--
** brotli_compress(dict:) **
brotli_compress(): Argument #1 ($data) not passed
** brotli_compress(data:, dict:) **
string(6) "string"
bool(true)
** brotli_compress(level:, dict:) **
brotli_compress(): Argument #1 ($data) not passed
** brotli_compress(mode:, dict:) **
brotli_compress(): Argument #1 ($data) not passed
** brotli_compress(data:, level:, dict:) **
string(6) "string"
bool(true)
** brotli_compress(data:, mode:, dict:) **
string(6) "string"
bool(true)
** brotli_compress(level:, mode:, dict:) **
brotli_compress(): Argument #1 ($data) not passed
** brotli_uncompress(dict:): false **
brotli_uncompress(): Argument #1 ($data) not passed
** brotli_uncompress(data:, dict:) **
string(6) "string"
bool(true)
** brotli_uncompress(length:, dict:) **
brotli_uncompress(): Argument #1 ($data) not passed
** brotli_uncompress(data: length:, dict:) **
string(6) "string"
bool(true)
===DONE===
