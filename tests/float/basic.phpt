--TEST--
Float Basic
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$nums = [
  0,     '0',    '0x0',   '00',
  0.0,   '0.0',  '0x0.0', '00.0',
  1.0,   '1.0',  '0x1.0', '01.0',
  1.25,  '1.25', '0x1.4', '01.2',
  8,     '8',    '0x8',   '08',
  8.25,  '8.25', '0x8.4', '08.2',
  'a',   'A',    'z',     'Z',
];

foreach ([0, 2, 8, 10, 16, 62] as $base) {
  echo "* base $base\n";
  foreach ($nums as $num) {
    echo var_export($num), ': ';
    try {
      $n = new \GMPi\Float($num, $base);
      // Number format to avoid lack of precision in high bases
      $f = $n->toString(10);
      if (strlen($f) > 10) {
        $f = rtrim(number_format($f, 10, '.', ''), '0.');
      }
      echo "$f\n";
    } catch (\TypeError $e) {
      echo $e->getMessage(), "\n";
    }
  }
}

--EXPECT--
* base 0
0: 0
'0': 0
'0x0': 0
'00': 0
0.0: 0
'0.0': 0
'0x0.0': 0
'00.0': 0
1.0: 1
'1.0': 1
'0x1.0': 1
'01.0': 1
1.25: 1.25
'1.25': 1.25
'0x1.4': 1.25
'01.2': 1.25
8: 8
'8': 8
'0x8': 8
'08': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
8.25: 8.25
'8.25': 8.25
'0x8.4': 8.25
'08.2': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'a': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'A': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'Z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
* base 2
0: 0
'0': 0
'0x0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'00': 0
0.0: 0
'0.0': 0
'0x0.0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'00.0': 0
1.0: 1
'1.0': 1
'0x1.0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'01.0': 1
1.25: 1.25
'1.25': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'0x1.4': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'01.2': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
8: 8
'8': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'0x8': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'08': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
8.25: 8.25
'8.25': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'0x8.4': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'08.2': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'a': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'A': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'Z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
* base 8
0: 0
'0': 0
'0x0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'00': 0
0.0: 0
'0.0': 0
'0x0.0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'00.0': 0
1.0: 1
'1.0': 1
'0x1.0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'01.0': 1
1.25: 1.25
'1.25': 1.328125
'0x1.4': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'01.2': 1.25
8: 8
'8': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'0x8': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'08': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
8.25: 8.25
'8.25': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'0x8.4': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'08.2': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'a': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'A': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'Z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
* base 10
0: 0
'0': 0
'0x0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'00': 0
0.0: 0
'0.0': 0
'0x0.0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'00.0': 0
1.0: 1
'1.0': 1
'0x1.0': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'01.0': 1
1.25: 1.25
'1.25': 1.25
'0x1.4': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'01.2': 1.2
8: 8
'8': 8
'0x8': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'08': 8
8.25: 8.25
'8.25': 8.25
'0x8.4': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'08.2': 8.2
'a': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'A': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'Z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
* base 16
0: 0
'0': 0
'0x0': 0
'00': 0
0.0: 0
'0.0': 0
'0x0.0': 0
'00.0': 0
1.0: 1
'1.0': 1
'0x1.0': 1
'01.0': 1
1.25: 1.25
'1.25': 1.14453125
'0x1.4': 1.25
'01.2': 1.125
8: 8
'8': 8
'0x8': 8
'08': 8
8.25: 8.25
'8.25': 8.14453125
'0x8.4': 8.25
'08.2': 8.125
'a': 10
'A': 10
'z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
'Z': Expected instance of GMPi\Integer, GMPi\Float, int, float, or a numeric string
* base 62
0: 0
'0': 0
'0x0': 3658
'00': 0
0.0: 0
'0.0': 0
'0x0.0': 3658
'00.0': 0
1.0: 1
'1.0': 1
'0x1.0': 3659
'01.0': 1
1.25: 1.25
'1.25': 1.0335587929
'0x1.4': 3659.064516129
'01.2': 1.0322580645
8: 8
'8': 8
'0x8': 3666
'08': 8
8.25: 8.25
'8.25': 8.0335587929
'0x8.4': 3666.064516129
'08.2': 8.0322580645
'a': 36
'A': 10
'z': 61
'Z': 35