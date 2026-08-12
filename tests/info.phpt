--TEST--
Test phpinfo() displays brotli info
--SKIPIF--
<?php if (!extension_loaded('brotli')) die('skip'); ?>
--FILE--
<?php
$ref = new ReflectionExtension('brotli');
ob_start();
$ref->info();
$info = ob_get_contents();
ob_end_clean();

// skip uninteresting lines
$lines = [];
foreach(explode("\n", $info) as $line) {
    if ($line && $line !== 'brotli') {
        $lines[] = $line;
    }
}

$verNum = '(([0-9]{1,2})\.([0-9]{1,2})\.([0-9]{1,2}))';

if (count($lines) >= 5) {
    echo preg_match('/^Extension\sversion\s\=\>\s'.$verNum.'$/', $lines[0]) ? "Ext version OK\n" : "Fail\n";

    $libBrotli = '(bundled|external)';
    $useDict = '(enabled|disabled)';
    $hasApcu = null;

    if (file_exists($configH = dirname(__DIR__) . '/config.h')) {
        $configH = file_get_contents($configH);
        $libBrotli = preg_match('/define\sUSE_BROTLI_BUNDLED\s1/', $configH) ? 'bundled' : 'external';
        $useDict = preg_match('/define\sUSE_BROTLI_DICTIONARY\s1/', $configH) ? 'enabled' : 'disabled';
        $hasApcu = preg_match('/define\sHAVE_APCU_SUPPORT\s1/', $configH) ? true : false;
    }

    echo preg_match('/^Brotli\slibrary\s\=\>\s'.$libBrotli.'$/', $lines[1]) ? "Bundled/external brotli OK\n" : "Fail\n";

    echo preg_match('/^Brotli\slibrary\sversion\s\=\>\s'.$verNum.'$/', $lines[2]) ? "Brotli version OK\n" : "Fail\n";

    echo preg_match('/^Dictionary\ssupport\s\=\>\s'.$useDict.'$/', $lines[3]) ? "Dictionary OK\n" : "Fail\n";

    if ($hasApcu === null) {
        echo "Apcu OK\n"; // just assume it is okay
    } else if ($hasApcu) {
        if (substr($lines[4], 0, 18) !== 'APCu serializer =>') {
            echo "Fail\n";
        } else {
            $fail = '';
            $value = substr($lines[4], 19);

            if (!extension_loaded('apcu')) {
                if ($value !== 'APCu extension not loaded') {
                    $fail .= 'should be not loaded ';
                }
            } else if ($value !== (ini_get('apc.serializer') === 'brotli' ? 'brotli active' : 'brotli inactive')) {
                $fail .= 'active/inactive mismatch ';
            }

            if (
                !isset($lines[5])
                ||
                !preg_match('/^APCu\sserializer\sinterface\sversion\s\=\>\s([0-9])/', $lines[5])
            ) {
                $fail .= 'ABI ';
            }

            echo !$fail ? "Apcu OK\n" : ("Fail: " . $fail . "");
        }
    } else {
        echo ($lines[4] == 'APCu serializer support => not built') ? "Apcu OK\n" : "Fail: not built\n";
    }

    $search = 'Built-in output compression exclusions => ';
    $mimeIndex = null;
    // could be 5 or 6 depending on apcu
    foreach([5, 6] as $index) {
        if (isset($lines[$index]) && substr($lines[$index], 0, 42) == $search) {
            $mimeIndex = $index;
        }
    }
    if ($mimeIndex) {
        $types = explode(', ', substr($lines[$mimeIndex], 42));
        $invalidTypes = [];
        foreach($types as $type) {
            if (!preg_match('/^([a-z]+)\/(([a-z0-9\-\.]+)|\*)$/', $type)) {
                $invalidTypes[] = $type;
            }
        }
        if ($invalidTypes) {
            echo "Fail: " . implode(", ", $invalidTypes) . "\n";
        } else {
            echo "MIMEs OK\n";
        }
    } else {
        echo "Fail\n";
    }
}
--EXPECTF--
Ext version OK
Bundled/external brotli OK
Brotli version OK
Dictionary OK
Apcu OK
MIMEs OK
