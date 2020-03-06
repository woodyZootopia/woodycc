#include "wdcc.h"

VarBlock *globals;

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
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
