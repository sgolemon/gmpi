--TEST--
Integer Power and Modulus
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$i = new \GMPi\Integer('12345678901234567890');
echo $i->pow(3), "\n";
echo $i ** 3, "\n";
echo $i->powMod('23456789012345678901', 12345), "\n";
echo $i->mod(12345), "\n";
echo $i % 12345, "\n";
--EXPECT--
1881676372353657772490265749424677022198701224860897069000
1881676372353657772490265749424677022198701224860897069000
11205
4395
4395
