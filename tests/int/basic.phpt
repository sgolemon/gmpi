--TEST--
Integer Basic
--SKIPIF--
<?php
if (!extension_loaded('gmpi')) echo 'skip';
--FILE--
<?php

$nums = [
  0.0,
  0, '0', '0x0', '00',
  1, '1', '0x1', '01',
  2, '2', '0x2', '02',
  8, '8', '0x8', '08',
  'a', 'A', 'z', 'Z',
];

foreach ([0, 2, 8, 10, 16, 62] as $base) {
  echo "* base $base\n";
  foreach ($nums as $num) {
    echo var_export($num), ': ';
    try {
      $n = new \GMPi\Integer($num, $base);
      echo $n, "\n";
    } catch (\TypeError $e) {
      echo $e->getMessage(), "\n";
    }
  }
}

--EXPECT--
* base 0
0.0: GMPi\Integer::__construct expects an integer or numeric string, float provided
0: 0
'0': 0
'0x0': 0
'00': 0
1: 1
'1': 1
'0x1': 1
'01': 1
2: 2
'2': 2
'0x2': 2
'02': 2
8: 8
'8': 8
'0x8': 8
'08': Initial value invalid for provided base: 0
'a': Initial value invalid for provided base: 0
'A': Initial value invalid for provided base: 0
'z': Initial value invalid for provided base: 0
'Z': Initial value invalid for provided base: 0
* base 2
0.0: GMPi\Integer::__construct expects an integer or numeric string, float provided
0: 0
'0': 0
'0x0': Initial value invalid for provided base: 2
'00': 0
1: 1
'1': 1
'0x1': Initial value invalid for provided base: 2
'01': 1
2: 2
'2': Initial value invalid for provided base: 2
'0x2': Initial value invalid for provided base: 2
'02': Initial value invalid for provided base: 2
8: 8
'8': Initial value invalid for provided base: 2
'0x8': Initial value invalid for provided base: 2
'08': Initial value invalid for provided base: 2
'a': Initial value invalid for provided base: 2
'A': Initial value invalid for provided base: 2
'z': Initial value invalid for provided base: 2
'Z': Initial value invalid for provided base: 2
* base 8
0.0: GMPi\Integer::__construct expects an integer or numeric string, float provided
0: 0
'0': 0
'0x0': Initial value invalid for provided base: 8
'00': 0
1: 1
'1': 1
'0x1': Initial value invalid for provided base: 8
'01': 1
2: 2
'2': 2
'0x2': Initial value invalid for provided base: 8
'02': 2
8: 8
'8': Initial value invalid for provided base: 8
'0x8': Initial value invalid for provided base: 8
'08': Initial value invalid for provided base: 8
'a': Initial value invalid for provided base: 8
'A': Initial value invalid for provided base: 8
'z': Initial value invalid for provided base: 8
'Z': Initial value invalid for provided base: 8
* base 10
0.0: GMPi\Integer::__construct expects an integer or numeric string, float provided
0: 0
'0': 0
'0x0': Initial value invalid for provided base: 10
'00': 0
1: 1
'1': 1
'0x1': Initial value invalid for provided base: 10
'01': 1
2: 2
'2': 2
'0x2': Initial value invalid for provided base: 10
'02': 2
8: 8
'8': 8
'0x8': Initial value invalid for provided base: 10
'08': 8
'a': Initial value invalid for provided base: 10
'A': Initial value invalid for provided base: 10
'z': Initial value invalid for provided base: 10
'Z': Initial value invalid for provided base: 10
* base 16
0.0: GMPi\Integer::__construct expects an integer or numeric string, float provided
0: 0
'0': 0
'0x0': Initial value invalid for provided base: 16
'00': 0
1: 1
'1': 1
'0x1': Initial value invalid for provided base: 16
'01': 1
2: 2
'2': 2
'0x2': Initial value invalid for provided base: 16
'02': 2
8: 8
'8': 8
'0x8': Initial value invalid for provided base: 16
'08': 8
'a': 10
'A': 10
'z': Initial value invalid for provided base: 16
'Z': Initial value invalid for provided base: 16
* base 62
0.0: GMPi\Integer::__construct expects an integer or numeric string, float provided
0: 0
'0': 0
'0x0': 3658
'00': 0
1: 1
'1': 1
'0x1': 3659
'01': 1
2: 2
'2': 2
'0x2': 3660
'02': 2
8: 8
'8': 8
'0x8': 3666
'08': 8
'a': 36
'A': 10
'z': 61
'Z': 35
