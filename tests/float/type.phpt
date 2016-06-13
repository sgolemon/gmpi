--TEST--
Float Type Conversion
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$f = new \GMPi\Float(123);
$i = $f->toInteger();
var_dump($i, $f);
var_dump($f->cmp(123));
var_dump($f->cmp(123.0));
var_dump($f->cmp($i));
var_dump($f->cmp($i->toFloat()));
--EXPECTF--
object(GMPi\Integer)#%d (2) {
  ["dec"]=>
  string(3) "123"
  ["hex"]=>
  string(2) "7b"
}
object(GMPi\Float)#%d (2) {
  ["dec"]=>
  string(3) "123"
  ["hex"]=>
  string(2) "7b"
}
int(0)
int(0)
int(0)
int(0)
