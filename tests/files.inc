<?php
function roundtrip_files()
{
    $libdir = dirname(__FILE__) . '/../brotli/';
    $files = array(
        $libdir . 'tests/testdata/alice29.txt',
        $libdir . 'tests/testdata/asyoulik.txt',
        $libdir . 'tests/testdata/lcet10.txt',
        $libdir . 'tests/testdata/plrabn12.txt',
        $libdir . '/c/enc/encode.c',
        $libdir . '/c/common/dictionary.h',
        $libdir . '/c/dec/decode.c',
    );
    foreach ($files as $file) {
        if (!file_exists($file)) {
            return false;
        }
    }
    return $files;
}

function compatibility_files()
{
    $libdir = dirname(__FILE__) . '/../brotli/';
    $files = glob($libdir . 'tests/testdata/*.compressed*');
    if (empty($files)) {
        return false;
    }
    return $files;
}
