#include "wdcc.h"
#include <stdio.h>

int foo() { printf("GOOOOOD\n"); }

int bar(int x, int y) { printf("%d\n", x + y); }

int bag(int x, int y, int z, int w) { printf("%d\n", x * y + z * w); }
