--TEST--
Integer Addition and subtraction
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$n = new \GMPi\Integer('10000000000000000', 16); // 2^64
echo $n->toString(), "\n";
echo $n->toString(16), "\n";
echo $n->add($n)->toString(16), "\n";
echo ($n + $n)->toString(16), "\n";
echo $n->sub('0x100000000')->toString(16), "\n";
echo ($n - '0x100000000')->toString(16), "\n";
echo $n->add(1)->sub(2)->toString(16), "\n";
echo ($n + 1 - 2)->toString(16), "\n";

--EXPECT--
18446744073709551616
10000000000000000
20000000000000000
20000000000000000
ffffffff00000000
ffffffff00000000
ffffffffffffffff
ffffffffffffffff
