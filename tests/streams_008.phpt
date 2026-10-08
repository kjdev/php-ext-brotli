--TEST--
compress.brotli streams report a successful close
--FILE--
<?php
$file = dirname(__FILE__) . '/streams_008.out';
$plain = dirname(__FILE__) . '/streams_008.txt';

// As of PHP 8.6, fclose(), file_put_contents() and copy() return false when
// closing the stream fails, so the close handler has to report success.
$data = str_repeat('abcd', 1024);

echo "Compress\n";

$fp = fopen('compress.brotli://' . $file, 'w');
var_dump($fp !== false);
var_dump(fwrite($fp, $data) === strlen($data));
var_dump(fclose($fp));

echo "Decompress\n";

$fp = fopen('compress.brotli://' . $file, 'r');
var_dump($fp !== false);
var_dump(stream_get_contents($fp) === $data);
var_dump(fclose($fp));

echo "Copy\n";

file_put_contents($plain, $data);
var_dump(copy($plain, 'compress.brotli://' . $file));
var_dump(file_get_contents('compress.brotli://' . $file) === $data);

@unlink($file);
@unlink($plain);
?>
===DONE===
--EXPECT--
Compress
bool(true)
bool(true)
bool(true)
Decompress
bool(true)
bool(true)
bool(true)
Copy
bool(true)
bool(true)
===DONE===
