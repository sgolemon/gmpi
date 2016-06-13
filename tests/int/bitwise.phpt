--TEST--
Integer Bitwise Operations
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$i = new \GMPi\Integer('0x12345678901234567890');
echo $i->and('0xffff0000')->toString(16), "\n";
echo $i->or('0xffff0000')->toString(16), "\n";
echo $i->xor('0xffff0000')->toString(16), "\n";
--EXPECT--
34560000
123456789012ffff7890
123456789012cba97890
