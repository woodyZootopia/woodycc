#include "wdcc.h"

LVar *locals;

int main(int argc, char **argv) {
    locals = malloc(sizeof(LVar));
    if (argc != 2) {
        return 1;
    }

    if (!strcmp(argv[1], "-test")) {
        runtest();
        return 0;
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    tokenize(argv[1]);
    program();

    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
    }

    return 0;
}
