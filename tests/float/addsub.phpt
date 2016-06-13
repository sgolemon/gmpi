--TEST--
Integer Addition and subtraction
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

ini_set('gmpi.precision', 100);
ini_set('gmpi.padding', 20);

$n = new \GMPi\Float('0x100000000.00000004');
echo $n->toString(16), "\n";
echo $n->add($n)->toString(16), "\n";
echo ($n + $n)->toString(16), "\n";
echo $n->sub('0x100000000')->toString(16), "\n";
echo ($n - '0x100000000')->toString(16), "\n";
echo $n->add(1)->sub(2)->toString(16), "\n";
echo ($n + 1 - 2)->toString(16), "\n";

--EXPECT--
100000000.00000004
200000000.00000008
200000000.00000008
0.00000004
0.00000004
ffffffff.00000004
ffffffff.00000004
