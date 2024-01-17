--TEST--
Test brotli_compress_add() functionality
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip need ext/brotli');
if (PHP_VERSION_ID < 70000) die('skip need version: 7.0+');
?>
--FILE--
<?php

$brotliContext = brotli_compress_init();

$strings = [
  'Hello, how are you? How is it going?' . PHP_EOL,
  'I am fine thanks' . PHP_EOL,
  'Hello, how are you? How is it going?' . PHP_EOL,
];

$compressed = '';
foreach ($strings as $string) {
  $compressedString = brotli_compress_add($brotliContext, $string);
  if (strlen($compressedString) === 0) {
    echo "Incremental compress failed\n";
  } elseif ($compressedString === $string) {
    // temporary only for test
    echo "Incremental compress failed to compress\n";
  } else {
    $compressed .= $compressedString;
  }
}

$compressed .= brotli_compress_add($brotliContext, '', BROTLI_FINISH);
if (brotli_uncompress($compressed) !== implode('', $strings)) {
  echo "Compression invalid\n";
} else {
  echo "OK\n";
}
?>
===DONE===
--EXPECTF--
OK
===DONE===
