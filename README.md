# Brotli Extension for PHP

[![Linux](https://github.com/kjdev/php-ext-brotli/actions/workflows/linux.yaml/badge.svg?branch=master)](https://github.com/kjdev/php-ext-brotli/actions/workflows/linux.yaml)
[![Windows](https://github.com/kjdev/php-ext-brotli/actions/workflows/windows.yaml/badge.svg?branch=master)](https://github.com/kjdev/php-ext-brotli/actions/workflows/windows.yaml)

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

php.ini:

```
extension=brotli.so
```

### Output handler option

Name                              | Default | Changeable
--------------------------------- | ------- | ----------
brotli.output\_compression        | 0       | PHP\_INI\_ALL
brotli.output\_compression\_level | 11      | PHP\_INI\_ALL
brotli.output\_compression\_dict  | ""      | PHP\_INI\_ALL

* brotli.output\_compression _boolean_

    Whether to transparently compress pages.
    If this option is set to "On" in php.ini or the Apache configuration,
    pages are compressed if the browser sends an "Accept-Encoding: br" header.
    "Content-Encoding: br" and "Vary: Accept-Encoding" headers are added to
    the output. In runtime, it can be set only before sending any output.

* brotli.output\_compression\_level _integer_

    Compression level used for transparent output compression.
    Specify a value between 0 to 11.
    The default value of `BROTLI_COMPRESS_LEVEL_DEFAULT` (11).

* brotli.output\_compression\_dict _string_

    Specifies the path to the compressed dictionary file to be
    used by the output handler.

    > can be used when `BROTLI_DICTIONARY_SUPPORT` is enabled

## Constant

Name                              | Description
--------------------------------- | -----------
BROTLI\_GENERIC                   | Generic compress mode value
BROTLI\_TEXT                      | Text compress mode value
BROTLI\_FONT                      | Font compress mode value
BROTLI\_COMPRESS\_LEVEL\_MIN      | Minimal compress level value
BROTLI\_COMPRESS\_LEVEL\_MAX      | Maximal compress level value
BROTLI\_COMPRESS\_LEVEL\_DEFAULT  | Default compress level value
BROTLI\_PROCESS                   | Incremental process mode value
BROTLI\_FLUSH                     | Incremental produce mode value
BROTLI\_FINISH                    | Incremental finalize mode value
BROTLI\_DICTIONARY\_SUPPORT       | Dictionary support value

> `BROTLI_DICTIONARY_SUPPORT` must be enabled
> with brotli library version 1.1.0 or higher

## Function

* brotli\_compress — Compress a string
* brotli\_uncompress — Uncompress a compressed string
* brotli\_compress\_init — Initialize an incremental compress context
* brotli\_compress\_add — Incrementally compress data
* brotli\_uncompress\_init — Initialize an incremental uncompress context
* brotli\_uncompress\_add — Incrementally uncompress data

---
### brotli\_compress — Compress a string

#### Description

``` php
brotli_compress ( string $data, int $level = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC, string|null $dict = null ): string|false
```

This function compress a string.

#### Parameters

* _data_

  The data to compress.

* _level_

  The higher the level, the slower the compression.
  (Defaults to `BROTLI_COMPRESS_LEVEL_DEFAULT`)

* _mode_

  The compression mode can be `BROTLI_GENERIC` (default),
  `BROTLI_TEXT` (for UTF-8 format text input) or `BROTLI_FONT` (for WOFF 2.0).

* _dict_

  The dictionary data.

  > can be used when `BROTLI_DICTIONARY_SUPPORT` is enabled

#### Return Values

The compressed string or FALSE if an error occurred.

---
### brotli\_uncompress — Uncompress a compressed string

#### Description

``` php
brotli_uncompress ( string $data, string|null $dict = null ): string|false
```

This function uncompress a compressed string.

#### Parameters

* _data_

  The data compressed by brotli\_compress().

* _dict_

  The dictionary data.

  > can be used when `BROTLI_DICTIONARY_SUPPORT` is enabled

#### Return Values

The original uncompressed data or FALSE on error.

---
### brotli\_compress\_init — Initialize an incremental compress context

#### Description

``` php
brotli_compress_init ( int $level = BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = BROTLI_GENERIC, string|null $dict = null ): Brotli\Compress\Context|false
```

Initialize an incremental compress context.

#### Parameters

* _level_

  The higher the level, the slower the compression.
  (Defaults to `BROTLI_COMPRESS_LEVEL_DEFAULT`)

* _mode_

  The compression mode can be `BROTLI_GENERIC` (default),
  `BROTLI_TEXT` (for UTF-8 format text input) or `BROTLI_FONT` (for WOFF 2.0).

* _dict_

  The dictionary data.

  > can be used when `BROTLI_DICTIONARY_SUPPORT` is enabled

#### Return Values

Returns a `Brotli\Compress\Context` instance on success,
or FALSE on failure.

---
### brotli\_compress\_add — Incrementally compress data

#### Description

``` php
brotli_compress_add ( Brotli\Compress\Context $context, string $data, $mode = BROTLI\_FLUSH ): string|false
```

Incrementally compress data.

#### Parameters

* _context_

  A context created with `brotli_compress_init()`.

* _data_

  A chunk of data to compress.

* _mode_

  One of `BROTLI_FLUSH` (default) and `BROTLI_PROCESS`, `BROTLI_FINISH`.

  `BROTLI_FINISH` to terminate with the last chunk of data.

#### Return Values

Returns a chunk of compressed data, or FALSE on failure.

---
### brotli\_uncompress\_init — Initialize an incremental uncompress context

#### Description

``` php
brotli_uncompress_init ( string|null $dict = null ): Brotli\UnCompress\Context|false
```

Initialize an incremental uncompress context.

#### Parameters

* _dict_

  The dictionary data.

  > can be used when `BROTLI_DICTIONARY_SUPPORT` is enabled

#### Return Values

Returns a `Brotli\UnCompress\Context` instance on success,
or FALSE on failure.

---
### brotli\_uncompress\_add — Incrementally uncompress data

#### Description

``` php
brotli_uncompress_add ( Brotli\UnCompress\Context $context, string $data, $mode = BROTLI\_FLUSH ): string|false
```

Incrementally uncompress data.

#### Parameters

* _context_

  A context created with `brotli_uncompress_init()`.

* _data_

  A chunk of compressed data.

* _mode_

  One of `BROTLI_FLUSH` (default) and `BROTLI_PROCESS`, `BROTLI_FINISH`.

  `BROTLI_FINISH` to terminate with the last chunk of data.

#### Return Values

Returns a chunk of uncompressed data, or FALSE on failure.

## Namespace

``` php
Namespace Brotli;

function compress( string $data, int $level = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC, string|null $dict = null ): string|false {}
function uncompress( string $data, string|null $dict = null ): string|false {}
function compress_init( int $level = \BROTLI_COMPRESS_LEVEL_DEFAULT, int $mode = \BROTLI_GENERIC, string|null $dict = null ): \Brotli\Compress\Context|false {}
function compress_add( \Brotli\Compress\Context $context, string $data, $mode = \BROTLI_FLUSH ): string|false {}
function uncompress_init(string|null $dict = null): \Brotli\UnCompress\Context|false {}
function uncompress_add( \Brotli\UnCompress\Context $context, string $data, int $mode = \BROTLI_FLUSH ): string|false {}
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
file_put_contents("compress.brotli:///path/to/data.br", $data);
readfile("compress.brotli:///path/to/data.br");
```

### Incrementally

```php
// compression
$resource = brotli_compress_init();
$compressed = '';
$compressed .= brotli_compress_add($resource, 'Hello, ', BROTLI_FLUSH);
$compressed .= brotli_compress_add($resource, 'World!', BROTLI_FLUSH);
$compressed .= brotli_compress_add($resource, '', BROTLI_FINISH);

echo brotli_uncompress($compressed), PHP_EOL; // Hello, World!

// uncompression
$resource = brotli_uncompress_init();
$uncompressed = '';
$uncompressed .= brotli_uncompress_add($resource, substr($compressed, 0, 5), BROTLI_FLUSH);
$uncompressed .= brotli_uncompress_add($resource, substr($compressed, 5), BROTLI_FLUSH);
$uncompressed .= brotli_uncompress_add($resource, '', BROTLI_FINISH);

echo $uncompressed, PHP_EOL; // Hello, World!
```

### Dictionary

```php
$data = '..';

// load dictionary data
$dict = file_get_contents('data.dict');

// basic
$compressed = brotli_compress(data: $data, dict: $dict);
$uncompressed = brotli_uncompress(data: $compressed, dict: $dict);

// incrementally
$context = brotli_compress_init(dict: $dict);
$compressed = '';
$compressed .= brotli_compress_add($context, $data);
$compressed .= brotli_compress_add($context, '', BROTLI_FINISH);

$context = brotli_uncompress_init(dict: $dict);
$uncompressed = '';
$uncompressed .= brotli_uncompress_add($context, $compressed, BROTLI_FLUSH);
$uncompressed .= brotli_uncompress_add($context, '', BROTLI_FINISH);

// streams
$ctx = stream_context_create([
    'brotli' => [
        'dict' => $dict,
    ],
]);

file_put_contents('compress.brotli:///path/to/data.br', $data, 0, $ctx);

$uncompressed = file_get_contents('compress.brotli:///path/to/data.br', false, $ctx);

// output handler
ini_set('brotli.output_compression_dict', __DIR__ . '/data.dict');
ini_set('brotli.output_compression', 'On');
// OR: ob_start('ob_brotli_handler');
echo ...;
```

> Experimental: [Compression Dictionary Transport](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Compression_dictionary_transport) support
>
> must be specified headers.
> - `Accept-Encoding: dcb`
> - `Available-Dictionary: :<base64-hash>:`
