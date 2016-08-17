# Brotli Extension for PHP

[![Build Status](https://travis-ci.org/kjdev/php-ext-brotli.png?branch=master)](https://travis-ci.org/kjdev/php-ext-brotli)

This extension allows Brotli compression.

Documentation for Brotli can be found at
[» https://github.com/google/brotli/](https://github.com/google/brotli/).

## Build

```
% git clone --recursive --depth=1 https://github.com/kjdev/php-ext-brotli.git
% cd php-ext-brotli
% phpize
% ./configure
% make
$ make install
```

## Configuration

brotli.ini:

```
extension=brotli.so
```

## Function

* brotli\_compress — Compress a string
* brotli\_uncompress — Uncompress a compressed string

## brotli\_compress — Compress a string

### Description

string **brotli\_compress** ( string _$data_ [, int _$quality_ = 11, int _$mode_ = -1 ] )

This function compress the given string using the ZLIB data format.

### Parameters

* _data_

  The data to compress.

* _quality_

  The higher the quality, the slower the compression.
  (Defaults to 11)

* _mode_

  The compression mode can be `BROTLI_GENERIC` (default),
  `BROTLI_TEXT` (for UTF-8 format text input) or `BROTLI_FONT` (for WOFF 2.0).

### Return Values

The compressed string or FALSE if an error occurred.

### brotli\_uncompress — Uncompress a compressed string

### Description

string **brotli\_uncompress** ( string _$data_ [, int _$length_ = 0 ] )

This function uncompress a compressed string.

### Parameters

* _data_

  The data compressed by brotli\_compress().

* _length_

  The maximum length of data to decode.

### Return Values

The original uncompressed data or FALSE on error.

## Examples

```
$compressed = brotli_compress('Compresstest');

$uncompressed = brotli_uncompress($compressed);

echo $uncompressed;
```
