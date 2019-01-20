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
echo $?

./wdcc "foobar(x){return x;} main(){return foobar(1);}" > tmp.s
gcc -o tmp tmp.s foo.o
printf "foobar(x){return x;} main(){return foobar(1);} => "
./tmp
echo $?

./wdcc "foobar(x,y){return x+y;} main(){return foobar(1,2);}" > tmp.s
gcc -o tmp tmp.s foo.o
printf "foobar(x,y){return x+y;} main(){return foobar(1,2);} => "
./tmp
echo $?

try 'foobar(x){if(x!=1) return x; return 0;} main(){return foobar(3);}' 3

try "fib(x){if(x==0)return 1;if(x==1)return 1; return fib(x-1)+fib(x-2);} main(){return fib(10);}" 89

try "powerroftwo(x){if(x==0)return 1; return 2*powerroftwo(x-1);} main(){return powerroftwo(5);}" 32

try "powerroftwo(x,y){while(x!=0){y=y*2;x=x-1;} return y;} main(){return powerroftwo(5,1);}" 32

echo OK
