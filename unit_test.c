#include "wdcc.h"
#include <stdio.h>
#include <stdlib.h>

int expect(int line, long expected, long actual) {
    if (expected == actual)
        return 0;
    fprintf(stderr, "%d: %ld expected, but got %ld\n", line, expected, actual);
    exit(1);
}

void test_vector() {
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);
    for (long i = 0; i < 100; i++) {
        // use long instead of i for 64 bit OS.
        vec_push(vec, (void *)i);
    }

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);
}

void test_map() {
    Map *map = new_map();
    expect(__LINE__, 0l, (long)map_get(map, "foo"));

    map_put(map, "foo", (void *)2l);
    expect(__LINE__, 2l, (long)map_get(map, "foo"));

    map_put(map, "bar", (void *)4l);
    expect(__LINE__, 4l, (long)map_get(map, "bar"));

    map_put(map, "foo", (void *)6l);
    expect(__LINE__, 6l, (long)map_get(map, "foo"));
}

void runtest() {
    test_vector();
    test_map();
    printf("OK\n");
}
