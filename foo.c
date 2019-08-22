#include "wdcc.h"
#include <stdio.h>

void foo() { printf("GOOOOOD\n"); }

void bar(int x, int y) { printf("%d\n", x + y); }

void bag(int x, int y, int z, int w) { printf("%d\n", x * z + y * w); }

void alloc4(int **p, int a, int b, int c, int d) {
    *p = calloc(4, sizeof(int));
    **p = a;
    *(*p + 1) = b;
    *(*p + 2) = c;
    *(*p + 3) = d;
}
