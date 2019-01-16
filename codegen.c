#include "wdcc.h"

void gen_lval(Node *node) {
    if (node->ty != ND_IDENT) {
        error2("left hand side is not a variable", 0);
    }

    int offset = ('z' - node->name + 1) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n"); // rax is pointer to the variable
}

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

    if (node->ty == ND_IDENT) {
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }

    if (node->ty == '=') {
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
    }

    if (node->ty == ND_IF) {
        gen(node->lhs->lhs);
        gen(node->lhs->rhs);
        printf("    pop rax\n");
        printf("    pop rdi\n");
        printf("    cmp rdi, rax\n");
        if (node->lhs->ty == ND_E) {
            printf("    jne .L2\n");
        } else if (node->lhs->ty == ND_NE) {
            printf("    je .L2\n");
        } else {
            error2("either == or != should be used for condition", 0);
        }
        gen(node->rhs);
        printf("    pop rax\n");
        printf(".L2:\n");
        return;
    }

    if (node->ty == ND_FUNC) {
        if(node->lhs != NULL){
            gen(node->lhs);
        }
        printf("    call %s\n", node->func_name);
        return;
    }

    if (node->ty == ',') {
        gen(node->lhs);
        printf("    pop rax\n");
        switch(node->val){
            case 0:
                printf("    mov rdi, rax\n");
                break;
            case 1:
                printf("    mov rsi, rax\n");
                break;
            case 2:
                printf("    mov rdx, rax\n");
                break;
            case 3:
                printf("    mov rcx, rax\n");
                break;
            case 4:
                printf("    mov r8, rax\n");
                break;
            case 5:
                printf("    mov r9, rax\n");
                break;
        }
        gen(node->rhs);
        if(node->rhs->ty==ND_NUM){
            printf("    pop rax\n");
            switch(node->val){
                case 0:
                    printf("    mov rsi, rax\n");
                    break;
                case 1:
                    printf("    mov rdx, rax\n");
                    break;
                case 2:
                    printf("    mov rcx, rax\n");
                    break;
                case 3:
                    printf("    mov r8, rax\n");
                    break;
                case 4:
                    printf("    mov r9, rax\n");
                    break;
            }
        }
        return;
    }

    // If none of the above, the node is polynomial

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->ty) {
    case '+':
        printf("    add rax, rdi\n");
        break;
    case '-':
        printf("    sub rax, rdi\n");
        break;
    case '*':
        printf("    mul rdi\n");
        break;
    case '/':
        printf("    mov rdx, 0\n");
        printf("    div rdi\n");
        break;
    }
    printf("    push rax\n");
}
