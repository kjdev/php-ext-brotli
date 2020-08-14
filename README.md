# Brotli Extension for PHP

[![Build Status](https://travis-ci.org/kjdev/php-ext-brotli.png?branch=master)](https://travis-ci.org/kjdev/php-ext-brotli)
[![Build status](https://ci.appveyor.com/api/projects/status/eeemvc8t3sc9i6e2/branch/master?svg=true)](https://ci.appveyor.com/project/kjdev/php-ext-brotli/branch/master)

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

To use the system library (using pkg-config)

``` bash
% ./configure --with-libbrotli
```

## Distribution binary packages

### Fedora / CentOS / RHEL

RPM packages of this extension are available in [» Remi's RPM repository](https://rpms.remirepo.net/) and are named **php-brotli**.

## Configuration

brotli.ini:

```
extension=brotli.so
```
$ phpenmod brotli
### Output handler option

Name                              | Default | Changeable
--------------------------------- | ------- | ----------
brotli.output\_compression        | 0       | PHP\_INI\_ALL
brotli.output\_compression\_level | -1      | PHP\_INI\_ALL

* brotli.output\_compression _boolean_

    Whether to transparently compress pages.
    If this option is set to "On" in php.ini or the Apache configuration,
    pages are compressed if the browser sends an "Accept-Encoding: br" header.
    "Content-Encoding: br" and "Vary: Accept-Encoding" headers are added to
    the output. In runtime, it can be set only before sending any output.

* brotli.output\_compression\_level _integer_

    Compression level used for transparent output compression.
    Specify a value between 0 to 11.
    The default value of -1 uses internally defined values (11).

> Available since PHP 5.4.0.

## Constant

Name                              | Description
--------------------------------- | -----------
BROTLI\_COMPRESS\_LEVEL\_MIN      | Minimal compress level value
BROTLI\_COMPRESS\_LEVEL\_MAX      | Maximal compress level value
BROTLI\_COMPRESS\_LEVEL\_DEFAULT  | Default compress level value

## Function

* brotli\_compress — Compress a string
* brotli\_uncompress — Uncompress a compressed string
* brotli\_compress\_init — Initialize an incremental compress context (PHP 7)
* brotli\_compress\_add — Incrementally compress data (PHP 7)
* brotli\_uncompress\_init — Initialize an incremental uncompress context (PHP 7)
* brotli\_uncompress\_add — Incrementally uncompress data (PHP 7)

---
### brotli\_compress — Compress a string

#### Description

string **brotli\_compress** ( string _$data_ [, int _$quality_ = 11, int _$mode_ = -1 ] )

This function compress a string.

#### Parameters

* _data_

  The data to compress.

* _quality_

  The higher the quality, the slower the compression.
  (Defaults to 11)

* _mode_

  The compression mode can be `BROTLI_GENERIC` (default),
  `BROTLI_TEXT` (for UTF-8 format text input) or `BROTLI_FONT` (for WOFF 2.0).

#### Return Values

The compressed string or FALSE if an error occurred.

---
### brotli\_uncompress — Uncompress a compressed string

#### Description

string **brotli\_uncompress** ( string _$data_ [, int _$length_ = 0 ] )

This function uncompress a compressed string.

#### Parameters

* _data_

  The data compressed by brotli\_compress().

* _length_

  The maximum length of data to decode.

#### Return Values

The original uncompressed data or FALSE on error.

---
### brotli\_compress\_init — Initialize an incremental compress context

#### Description

string **brotli\_compress\_init** ( void )

Initialize an incremental compress context. (PHP 7)

#### Return Values

Returns a brotli context resource (brotli.state) on success,
or FALSE on failure.

---
### brotli\_compress\_add — Incrementally compress data

#### Description

string **brotli\_compress\_add** ( resource _$context_, string _$data_ [, _$mode_ = BROTLI\_PROCESS ] )

Incrementally compress data. (PHP 7)

#### Parameters

* _context_

  A context created with `brotli_compress_init()`.

* _data_

  A chunk of data to compress.

* _mode_

  One of `BROTLI_PROCESS` (default), `BROTLI_FINISH`.

  `BROTLI_FINISH` to terminate with the last chunk of data.

#### Return Values

Returns a chunk of compressed data, or FALSE on failure.

---
### brotli\_uncompress\_init — Initialize an incremental uncompress context

#### Description

string **brotli\_uncompress\_init** ( void )

Initialize an incremental uncompress context. (PHP 7)

#### Return Values

Returns a brotli context resource (brotli.state) on success,
or FALSE on failure.

---
### brotli\_uncompress\_add — Incrementally uncompress data

#### Description

string **brotli\_uncompress\_add** ( resource _$context_, string _$data_ [, _$mode_ = BROTLI\_PROCESS ] )

Incrementally uncompress data. (PHP 7)

#### Parameters

* _context_

  A context created with `brotli_uncompress_init()`.

* _data_

  A chunk of compressed data.

* _mode_

  One of `BROTLI_PROCESS` (default), `BROTLI_FINISH`.

  `BROTLI_FINISH` to terminate with the last chunk of data.

#### Return Values

Returns a chunk of uncompressed data, or FALSE on failure.

## Namespace

```
Namespace Brotli;

function compress( $data [, $quality = 11, $mode = -1 ] )
function uncompress( $data [, $length = 0 ] )
function compress\_init()
function compress\_add( resource $context, string $data [, $mode = BROTLI\_PROCESS] )
function uncompress\_init()
function uncompress\_add( resource $context, string $data [, $mode = BROTLI\_PROCESS] )
```

alias functions..


## Streams

Brotli compression and uncompression are available using the
`compress.brotli://` stream prefix.

## Examples

``` php
$compressed = brotli_compress('Compresstest');

$uncompressed = brotli_uncompress($compressed);

echo $uncompressed;
```

### Output handler

``` php
ini_set('brotli.output_compression', 'On');
// OR
// ob_start('ob_brotli_handler');

echo ...;
```

> "Accept-Encoding: br" must be specified.

### Namespace

``` php
$data = \Brotli\compress('test');

\Brotli\uncompress($data);
```

### Streams

``` php
file_put_contents("compress.brotli:///patch/to/data.br", $data);
readfile("brotli.brotli:///patch/to/data.br");
```

### Incrementally

```php
// compression
$resource = brotli_compress_init();
$compressed = '';
$compressed .= brotli_compress_add($resource, 'Hello, ', BROTLI_PROCESS);
$compressed .= brotli_compress_add($resource, 'World!', BROTLI_PROCESS);
$compressed .= brotli_compress_add($resource, '', BROTLI_FINISH);

echo brotli_uncompress($compressed), PHP_EOL; // Hello, World!

// uncompression
$resource = brotli_uncompress_init();
$uncompressed = '';
$uncompressed .= brotli_uncompress_add($resource, substr($compressed, 0, 5), BROTLI_PROCESS);
$uncompressed .= brotli_uncompress_add($resource, substr($compressed, 5), BROTLI_PROCESS);
$uncompressed .= brotli_uncompress_add($resource, '', BROTLI_FINISH);

echo $uncompressed, PHP_EOL; // Hello, World!
```
