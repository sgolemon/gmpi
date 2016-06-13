--TEST--
Float Power
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

ini_set('gmpi.precision', 100);
ini_set('gmpi.padding', 20);

$f = new \GMPi\Float('1234567890.1234567890');
echo $f->pow(3), "\n";
echo $f ** 3, "\n";
--EXPECT--
1881676372353657772490265749.424677022199
1881676372353657772490265749.424677022199