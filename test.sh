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
try "main(){return 0;}" 0
try "main(){return 42;}" 42
try 'main(){return 5+20-4;}' 21
try 'main(){return 5+20 - 4;}' 21
try 'main(){return 12 + 34 - 5;} ' 41

try "main(){return 5+6*7;}" 47
try "main(){return 5*(9-6);}" 15
try "main(){return (3+5)/2;}" 4

try "main(){a+b;a=15;return 3;}" 3
try "main(){a+b;a=15;return a;}" 15
try "main(){a=15;b=3;a=a+b;return a;}" 18

try "main(){{a=3;b=a+3;}return b;}" 6

try "main(){a=2;b=2;if(a==b)b=3;return b;}" 3
try "main(){a=3;b=2;if(a==b)b=3;return b;}" 2
try "main(){a=2;b=2;if(a!=b)b=3;return b;}" 2
try "main(){a=3;b=2;if(a!=b)b=3;return b;}" 3


./wdcc "main(){foo(); return 0;}" > tmp.s
gcc -o tmp tmp.s foo.o
printf "main(){foo(); return 0;} => "
./tmp

./wdcc "main(){bar(1,2); return 0;}" > tmp.s
gcc -o tmp tmp.s foo.o
printf "main(){bar(1,2); return 0;} => "
./tmp

./wdcc "main(){bag(6,10,3,4); return 0;}" > tmp.s
gcc -o tmp tmp.s foo.o
printf "main(){bag(6,10,3,4); return 0;} => "
./tmp

./wdcc "foobar(){return 3+5;} main(){return foobar();}" > tmp.s
gcc -o tmp tmp.s foo.o
printf "foobar(){return 3+5;} main(){return foobar();} => "
./tmp

echo OK
