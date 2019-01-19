#include "wdcc.h"
#include <stdio.h>

void foo() { printf("GOOOOOD\n"); }

void bar(int x, int y) { printf("%d\n", x + y); }

void bag(int x, int y, int z, int w) { printf("%d\n", x * z + y * w); }
