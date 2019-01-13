#!/bin/bash
try() {
    input="$1"
    expected="$2"

    ./wdcc "$input" > tmp.s
    gcc -o tmp tmp.s
    if [ $? == 1 ]; then
        echo "ERROR: object file created, but compile error"
        exit 1
    fi
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "ERROR: \"$input\" expected $expected, but got $actual"
        exit 1
    fi
}

# return value should be less than 256
try 0 0
try 42 42
try '5+20-4' 21
try '5+20 - 4' 21
try ' 12 + 34 - 5 ' 41

try "5+6*7" 47
try "5*(9-6)" 15
try "(3+5)/2" 4

try "42;3" 3

try "a+b;a=15;3" 3
try "a+b;a=15;a" 15
try "a=15;b=3;a=a+b;a" 18

echo OK
