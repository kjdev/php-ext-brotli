--TEST--
Test compatibility
--SKIPIF--
--FILE--
<?php
if (!extension_loaded('brotli')) {
    dl('brotli.' . PHP_SHLIB_SUFFIX);
}

$dir = dirname(__FILE__) . '/../brotli/';
$files = array(
    'tests/testdata/alice29.txt',
    'tests/testdata/asyoulik.txt',
    'tests/testdata/lcet10.txt',
    'tests/testdata/plrabn12.txt',
    '/c/enc/encode.c',
    '/c/common/dictionary.h',
    '/c/dec/decode.c',
);

$qualities = array(1, 6, 9, 11);

foreach ($files as $filename) {
    foreach ($qualities as $quality) {
        echo 'Roundtrip testing file ', basename($filename), ' at quality ', $quality, PHP_EOL;

        $expected = $dir . $filename;

        if (file_exists($expected)) {
            $data = file_get_contents($expected);
            $expected_data = brotli_uncompress(brotli_compress($data, $quality));
            if ($data !== $expected_data) {
                echo "  NG\n";
                exit(1);
            } else {
                echo "  OK\n";
            }
        }
    }
}

--EXPECTF--
Roundtrip testing file alice29.txt at quality 1
  OK
Roundtrip testing file alice29.txt at quality 6
  OK
Roundtrip testing file alice29.txt at quality 9
  OK
Roundtrip testing file alice29.txt at quality 11
  OK
Roundtrip testing file asyoulik.txt at quality 1
  OK
Roundtrip testing file asyoulik.txt at quality 6
  OK
Roundtrip testing file asyoulik.txt at quality 9
  OK
Roundtrip testing file asyoulik.txt at quality 11
  OK
Roundtrip testing file lcet10.txt at quality 1
  OK
Roundtrip testing file lcet10.txt at quality 6
  OK
Roundtrip testing file lcet10.txt at quality 9
  OK
Roundtrip testing file lcet10.txt at quality 11
  OK
Roundtrip testing file plrabn12.txt at quality 1
  OK
Roundtrip testing file plrabn12.txt at quality 6
  OK
Roundtrip testing file plrabn12.txt at quality 9
  OK
Roundtrip testing file plrabn12.txt at quality 11
  OK
Roundtrip testing file encode.c at quality 1
  OK
Roundtrip testing file encode.c at quality 6
  OK
Roundtrip testing file encode.c at quality 9
  OK
Roundtrip testing file encode.c at quality 11
  OK
Roundtrip testing file dictionary.h at quality 1
  OK
Roundtrip testing file dictionary.h at quality 6
  OK
Roundtrip testing file dictionary.h at quality 9
  OK
Roundtrip testing file dictionary.h at quality 11
  OK
Roundtrip testing file decode.c at quality 1
  OK
Roundtrip testing file decode.c at quality 6
  OK
Roundtrip testing file decode.c at quality 9
  OK
Roundtrip testing file decode.c at quality 11
  OK
