#!/bin/bash
try() {
    input="$1"
    expected="$2"

    ./woodycc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "expected $expected, but got $actual"
        exit 1
    fi
}

try 0 0
try 42 42
try '5+20-4' 21
try '5+20 - 4' 21
try ' 12 + 34 - 5 ' 41

echo OK
