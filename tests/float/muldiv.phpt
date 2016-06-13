--TEST--
Integer Multiplication and Division
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

ini_set('gmpi.precision', 75);
ini_set('gmpi.padding', 20);

$n = new \GMPi\Float('100000000.4', 16);
echo $n->toString(), "\n";
echo $n->toString(16), "\n";
echo $n->mul($n)->toString(16), "\n";
echo ($n * $n)->toString(16), "\n";
echo $n->div('0x100000000')->toString(16), "\n";
echo ($n / '0x100000000')->toString(16), "\n";
echo $n->mul(2)->div(4)->toString(16), "\n";
echo ($n * 2 / 4)->toString(16), "\n";

--EXPECT--
4294967296.25
100000000.4
10000000080000000.1
10000000080000000.1
1.000000004
1.000000004
80000000.2
80000000.2
