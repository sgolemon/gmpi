--TEST--
Type Conversion
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$i = new \GMPi\Integer(123);
$f = $i->toFloat();
var_dump($i, $f);
var_dump($i->cmp(123));
var_dump($i->cmp(123.0));
var_dump($i->cmp($f));
var_dump($i->cmp($f->toInteger()));
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
