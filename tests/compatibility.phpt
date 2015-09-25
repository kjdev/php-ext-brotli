--TEST--
Test compatibility
--SKIPIF--
--FILE--
<?php
if (!extension_loaded('brotli')) {
    dl('brotli.' . PHP_SHLIB_SUFFIX);
}

$dir = __DIR__ . '/../brotli/tests/testdata/*.compressed*';

foreach (glob($dir) as $filename) {
    echo 'Testing decompression of file ', basename($filename), PHP_EOL;

    $split =  explode('.compressed', $filename);
    $expected = $split[0];
    $quality = -1;

    if (isset($split[1])) {
        $quality = (int)$split[1];
    }

    if (file_exists($expected)) {
        $data = file_get_contents($expected);
        $compressed_data = file_get_contents($filename);

        $expected_data = brotli_uncompress($compressed_data);
        if ($data !== $expected_data) {
            echo "  read uncompressed .. NG\n";
            exit(1);
        } else {
            echo "  read uncompressed .. OK\n";
        }

        $expected_data = brotli_compress($data, $quality);
        if (!$expected_data) {
            echo "  compressed        .. NG\n";
            exit(1);
        } else {
            echo "  compressed        .. OK\n";
        }

        $expected_data = brotli_uncompress($compressed_data);
        if ($data !== $expected_data) {
            echo "  uncompressed      .. NG\n";
            exit(1);
        } else {
            echo "  uncompressed      .. OK\n";
        }

    }
}

--EXPECTF--
Testing decompression of file 10x10y.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file 64x.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file alice29.txt.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file asyoulik.txt.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file backward65536.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file compressed_file.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file compressed_repeated.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.00
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.01
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.02
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.03
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.04
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.05
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.06
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.07
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.08
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.09
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.10
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.11
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.12
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.13
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.14
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.15
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.16
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.17
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file empty.compressed.18
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file lcet10.txt.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file mapsdatazrh.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file monkey.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file plrabn12.txt.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file quickfox.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file quickfox_repeated.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file random_org_10k.bin.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file ukkonooa.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file x.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file x.compressed.00
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file x.compressed.01
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file x.compressed.02
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file x.compressed.03
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file xyzzy.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
Testing decompression of file zeros.compressed
  read uncompressed .. OK
  compressed        .. OK
  uncompressed      .. OK
