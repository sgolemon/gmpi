--TEST--
Integer comparison
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$i = new \GMPi\Integer(123);
$f = new \GMPi\Float(123.0);
var_dump($i < 125);
var_dump($i > 125);
var_dump($i <=> 125);
var_dump($i <=> 123);
var_dump($i <=> $f);
--EXPECT--
bool(true)
bool(false)
int(-1)
int(0)
int(0)
