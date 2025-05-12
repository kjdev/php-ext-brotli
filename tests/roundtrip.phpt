--TEST--
Test roundtrip
--SKIPIF--
<?php
if (!extension_loaded('brotli')) {
    die('skip');
}
include dirname(__FILE__) . '/files.inc';
if (roundtrip_files() === false) {
    die('skip');
}
?>
--FILE--
<?php
include dirname(__FILE__) . '/files.inc';
$files = roundtrip_files();

$qualities = array(BROTLI_COMPRESS_LEVEL_MIN, 6, 9, BROTLI_COMPRESS_LEVEL_MAX);

foreach ($files as $filename) {
    foreach ($qualities as $level) {
        echo 'Roundtrip testing file ', basename($filename), ' at level ', $level, PHP_EOL;

        $expected = $filename;

        if (file_exists($expected)) {
            $data = file_get_contents($expected);
            $expected_data = brotli_uncompress(brotli_compress($data, $level));
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
Roundtrip testing file alice29.txt at level 0
  OK
Roundtrip testing file alice29.txt at level 6
  OK
Roundtrip testing file alice29.txt at level 9
  OK
Roundtrip testing file alice29.txt at level 11
  OK
Roundtrip testing file asyoulik.txt at level 0
  OK
Roundtrip testing file asyoulik.txt at level 6
  OK
Roundtrip testing file asyoulik.txt at level 9
  OK
Roundtrip testing file asyoulik.txt at level 11
  OK
Roundtrip testing file lcet10.txt at level 0
  OK
Roundtrip testing file lcet10.txt at level 6
  OK
Roundtrip testing file lcet10.txt at level 9
  OK
Roundtrip testing file lcet10.txt at level 11
  OK
Roundtrip testing file plrabn12.txt at level 0
  OK
Roundtrip testing file plrabn12.txt at level 6
  OK
Roundtrip testing file plrabn12.txt at level 9
  OK
Roundtrip testing file plrabn12.txt at level 11
  OK
Roundtrip testing file encode.c at level 0
  OK
Roundtrip testing file encode.c at level 6
  OK
Roundtrip testing file encode.c at level 9
  OK
Roundtrip testing file encode.c at level 11
  OK
Roundtrip testing file dictionary.h at level 0
  OK
Roundtrip testing file dictionary.h at level 6
  OK
Roundtrip testing file dictionary.h at level 9
  OK
Roundtrip testing file dictionary.h at level 11
  OK
Roundtrip testing file decode.c at level 0
  OK
Roundtrip testing file decode.c at level 6
  OK
Roundtrip testing file decode.c at level 9
  OK
Roundtrip testing file decode.c at level 11
  OK
