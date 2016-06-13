--TEST--
Integer Multiplication and Division
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

ini_set('gmpi.precision', 20);
ini_set('gmpi.padding', 20);

$n = new \GMPi\Integer('10000000000000000', 16); // 2^64
echo $n->toString(), "\n";
echo $n->toString(16), "\n";
echo $n->mul($n)->toString(16), "\n";
echo ($n * $n)->toString(16), "\n";
echo $n->div('0x100000000')->toString(16), "\n";
echo ($n / '0x100000000')->toString(16), "\n";
echo $n->mul(2)->div(4)->toString(16), "\n";
echo ($n * 2 / 4)->toString(16), "\n";

--EXPECT--
18446744073709551616
10000000000000000
100000000000000000000000000000000
100000000000000000000000000000000
100000000
100000000
8000000000000000
8000000000000000
