--TEST--
checks brotli compression output size is always the same
--SKIPIF--
<?php
if (!extension_loaded('brotli')) die('skip');
?>
--CGI--
--FILE--
<?php

ini_set('brotli.output_compression', 1);
ini_set('brotli.output_compression_level', '9');

$lens = array();

for ( $i=0 ; $i < 100 ; $i++ ) {
  ob_start();
  phpinfo();
  $html = ob_get_clean();

  $len = strlen($html);

  $lens[$len] = $len;
}

$lens = array_values($lens);

echo "Compressed Lengths\n";

var_dump($lens); // show lengths to help triage in case of failure

// expected headers since its CGI

?>
--EXPECTF--
%s
array(1) {
  [0]=>
  int(%d)
}
