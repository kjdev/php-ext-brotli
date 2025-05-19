--TEST--
use dictionary: incremental
--SKIPIF--
<?php
if (BROTLI_DICTIONARY_SUPPORT === false) die('skip dictionary not supported');
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
$dictionary = file_get_contents(dirname(__FILE__) . '/data.dict');

$modeTypes = [
  'BROTLI_PROCESS' => BROTLI_PROCESS,
  'BROTLI_FLUSH' => BROTLI_FLUSH,
];

foreach ($modeTypes as $modeTypeKey => $modeType) {
    $compressed = '';

    $context = brotli_compress_init(BROTLI_COMPRESS_LEVEL_DEFAULT,
                                    BROTLI_GENERIC,
                                    $dictionary);

    foreach (str_split($data, 6) as $var) {
        $compressed .= brotli_compress_add($context, $var, $modeType);
    }
    $compressed .= brotli_compress_add($context, '', BROTLI_FINISH);

    if ($data === brotli_uncompress($compressed, $dictionary)) {
        echo "OK: {$modeTypeKey}\n";
    } else {
        echo "ERROR: {$modeTypeKey}\n";
    }

    $out = '';
    $context = brotli_uncompress_init($dictionary);
    foreach (str_split($compressed, 6) as $var) {
        $out .= brotli_uncompress_add($context, $var, $modeType);
    }
    $out .= brotli_uncompress_add($context, '', BROTLI_FINISH);

    if ($data === $out) {
        echo "Ok: {$modeTypeKey}\n";
    } else {
        echo "Error: {$modeTypeKey}\n";
    }
}
?>
===DONE===
--EXPECT--
OK: BROTLI_PROCESS
Ok: BROTLI_PROCESS
OK: BROTLI_FLUSH
Ok: BROTLI_FLUSH
===DONE===
