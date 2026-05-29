--TEST--
compress.brotli streams read with chunk size exceeding internal buffer
--FILE--
<?php
$file = dirname(__FILE__) . '/streams_007.out';

// PHP_BROTLI_BUFFER_SIZE is 1 << 19 (524288). Use a payload that does not
// compress (so the on-disk size also exceeds the internal buffer) and a read
// chunk larger than that, exercising the NEEDS_MORE_INPUT path with
// count > PHP_BROTLI_BUFFER_SIZE.
$chunk = 1024 * 1024; // 1 MiB > 524288
$data = random_bytes(1024 * 1024); // uncompressible payload
var_dump(strlen($data) > 524288);

echo "Compress\n";
$dst = fopen('compress.brotli://' . $file, 'wb');
var_dump(fwrite($dst, $data) === strlen($data));
var_dump(fclose($dst));
var_dump(filesize($file) > 524288);

echo "Decompress\n";
$fp = fopen('compress.brotli://' . $file, 'rb');
stream_set_chunk_size($fp, $chunk);
$out = stream_get_contents($fp);
var_dump(fclose($fp));
var_dump(strlen($out) === strlen($data));
var_dump($out === $data);

@unlink($file);
?>
===DONE===
--EXPECTF--
bool(true)
Compress
bool(true)
bool(true)
bool(true)
Decompress
bool(true)
bool(true)
bool(true)
===DONE===
