#include "wdcc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    if (!strcmp(argv[1], "-test")) {
        runtest();
        return 0;
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    tokenize(argv[1]);
    program();

    // prologue
    printf("    push rbp\n");       // base pointer stacked
    printf("    mov rbp, rsp\n");   // rsp is new base pointer
    printf("    sub rsp, 208\n\n"); // 208=26*8 bytes allocated

    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
        printf("    pop rax\n\n");
    }

    // epilogue
    printf("    mov rsp, rbp\n"); // freeing the local variables
    printf("    pop rbp\n");      // base pointer is back

    printf("    ret\n"); // return rax
    return 0;
}
