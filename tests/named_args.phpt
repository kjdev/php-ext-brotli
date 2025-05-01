--TEST--
use named arguments
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80000) die("skip requires PHP 8.0+");
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

$quality = BROTLI_COMPRESS_LEVEL_MAX;
$mode = BROTLI_TEXT;

echo "** brotli_compress() **\n";
try {
    var_dump(gettype(brotli_compress()));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(data:) **\n";
try {
    var_dump(gettype(brotli_compress(data: $data)));
    var_dump(brotli_uncompress(brotli_compress(data: $data)) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(quality:) **\n";
try {
    var_dump(gettype(brotli_compress(quality: $quality)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(mode:) **\n";
try {
    var_dump(gettype(brotli_compress(mode: $mode)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(data:, quality:) **\n";
try {
    var_dump(gettype(brotli_compress(data: $data, quality: $quality)));
    var_dump(brotli_uncompress(brotli_compress(data: $data, quality: $quality)) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(data:, mode:) **\n";
try {
    var_dump(gettype(brotli_compress(data: $data, mode: $mode)));
    var_dump(brotli_uncompress(brotli_compress(data: $data, mode: $mode)) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_compress(quality:, mode:) **\n";
try {
    var_dump(gettype(brotli_compress(quality: $quality, mode: $mode)));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

$compressed = brotli_compress(data: $data);

echo "** brotli_uncompress(): false **\n";
try {
    var_dump(gettype(brotli_uncompress()));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_uncompress(data:) **\n";
try {
    var_dump(gettype(brotli_uncompress(data: $compressed)));
    var_dump(brotli_uncompress(data: $compressed) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_uncompress(length:) **\n";
try {
    var_dump(gettype(brotli_uncompress(length: strlen($compressed))));
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}

echo "** brotli_uncompress(data: length:) **\n";
try {
    var_dump(gettype(brotli_uncompress(data: $compressed, length: strlen($compressed))));
    var_dump(brotli_uncompress(data: $compressed, length: strlen($compressed)) === $data);
} catch (Error $e) {
    echo $e->getMessage(), PHP_EOL;
}
?>
===DONE===
--EXPECTF--
** brotli_compress() **
brotli_compress() expects at least 1 argument, 0 given
** brotli_compress(data:) **
string(6) "string"
bool(true)
** brotli_compress(quality:) **
brotli_compress(): Argument #1 ($data) not passed
** brotli_compress(mode:) **
brotli_compress(): Argument #1 ($data) not passed
** brotli_compress(data:, quality:) **
string(6) "string"
bool(true)
** brotli_compress(data:, mode:) **
string(6) "string"
bool(true)
** brotli_compress(quality:, mode:) **
brotli_compress(): Argument #1 ($data) not passed
** brotli_uncompress(): false **
brotli_uncompress() expects at least 1 argument, 0 given
** brotli_uncompress(data:) **
string(6) "string"
bool(true)
** brotli_uncompress(length:) **
brotli_uncompress(): Argument #1 ($data) not passed
** brotli_uncompress(data: length:) **
string(6) "string"
bool(true)
===DONE===
