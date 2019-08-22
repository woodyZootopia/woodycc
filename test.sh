#!/bin/bash
try() {
    input="$1"
    expected="$2"
    object_file="$3"

    ./wdcc "$input" > tmp.s
    if [ $# == 3 ]; then
        gcc -o tmp tmp.s $3
    else
        gcc -o tmp tmp.s
    fi
    if [ $? == 1 ]; then
        echo "ERROR: object file created, but compile error"
        exit 1
    fi
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input
$expected"
    else
        echo "ERROR: \"$input\" expects $expected, but got $actual"
        exit 1
    fi
}

# return value should be less than 256
# tests basic arithmetics
echo "LINENO:$LINENO"
try "int main(){return 0;}" 0
try "int main(){return 42;}" 42
try 'int main(){return 5+20-4;}' 21
try 'int main(){return 5+20 - 4;}' 21
try 'int main(){return 12 + 34 - 5;} ' 41

echo "LINENO:$LINENO"
try "int main(){return 5+6*7;}" 47
try "int main(){return 5*(9-6);}" 15
try "int main(){return (3+5)/2;}" 4

echo "LINENO:$LINENO"
try "int main(){int a; int b; a+b;a=15;return 3;}" 3
try "int main(){int a; int b; a+b;a=15;return a;}" 15
try "int main(){int a; int b; a=15;b=3;a=a+b;return a;}" 18

echo "LINENO:$LINENO"
try "int main(){int a; int b; {a=3;b=a+3;}return b;}" 6

echo "LINENO:$LINENO"
try "int main(){int a; int b; a=2;b=2;if(a==b)b=3;return b;}" 3
try "int main(){int a; int b; a=3;b=2;if(a==b)b=3;return b;}" 2
try "int main(){int a; int b; a=2;b=2;if(a!=b)b=3;return b;}" 2
try "int main(){int a; int b; a=3;b=2;if(a!=b)b=3;return b;}" 3


# test printf function(external function)
echo "LINENO:$LINENO"
try "int main(){foo(); return 0;}" 0 "foo.o"
try "int main(){bar(1,2); return 0;}" 0 "foo.o"
try "int main(){bag(6,10,3,4); return 0;}" 0 "foo.o"

# test function definition
echo "LINENO:$LINENO"
try "int foobar(){return 3+5;} int main(){return foobar();}" 8 "foo.o"
try "int foobar(int x){return x;} int main(){return foobar(1);}" 1 "foo.o"
try "int foobar(int x,int y){return x+y;} int main(){return foobar(1,2);}" 3 "foo.o"
try "int foobar(int x,int y){return x+y;} int main(){int a; a = 2;return foobar(1,a);}" 3 "foo.o"
try 'int foobar(int x){if(x!=1) return x; return 0;} int main(){return foobar(3);}' 3
try "int fib(int x){if(x==0)return 1;if(x==1)return 1; return fib(x-1)+fib(x-2);} int main(){return fib(10);}" 89
try "int powerroftwo(int x){if(x==0)return 1; return 2*powerroftwo(x-1);} int main(){return powerroftwo(5);}" 32
try "int powerroftwo(int x,int y){while(x!=0){y=y*2;x=x-1;} return y;} int main(){return powerroftwo(5,1);}" 32

# test multiple character local variable
echo "LINENO:$LINENO"
try "int main(){int foo; int bar; foo=2;bar=3; return foo+bar;}" 5

# test pointer
echo "LINENO:$LINENO"
try "int main(){int x; int y; x=3; y=&x; return *y;}" 3
try "int main(){int x; int *y; y=&x; *y=3; return x;}" 3
# pointer arithmetic
try "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p; return *q;}" 1 foo.o
try "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 1; return *q;}" 2 foo.o
try "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *q;}" 4 foo.o
try "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 3; return *q;}" 8 foo.o
try "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 3; q = q - 1; return *q;}" 4 foo.o
echo PASSED ALL TESTS!
