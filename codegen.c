#include "wdcc.h"

int jump_num = 2;

void gen_lval(Node *node) {
    if (node->ty != ND_LVAR) {
        error2("left hand side is not a variable", 0);
    }

    int offset = node->offset;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n"); // now rax is the pointer to the variable
}

void gen_arg(Node *node) {
    int depth = 0;
    Node *tmp;
    if (node->ty == ND_LVAR) {
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        return;
    }
    for (tmp = node; tmp->ty != ND_LVAR; tmp = tmp->rhs) {
        gen_lval(tmp->lhs);
        printf("    pop rax\n");
        if (depth == 0) {
            printf("    mov [rax], rdi\n");
        }
        if (depth == 1) {
            printf("    mov [rax], rsi\n");
        }
        if (depth == 2) {
            printf("    mov [rax], rdx\n");
        }
        if (depth == 3) {
            printf("    mov [rax], rcx\n");
        }
        if (depth == 4) {
            printf("    mov [rax], r8\n");
        }
        if (depth == 5) {
            printf("    mov [rax], r9\n");
        }
        depth++;
    }
    gen_lval(tmp); // tmp is already the last argument
    printf("    pop rax\n");
    if (depth == 1) {
        printf("    mov [rax], rsi\n");
    }
    if (depth == 2) {
        printf("    mov [rax], rdx\n");
    }
    if (depth == 3) {
        printf("    mov [rax], rcx\n");
    }
    if (depth == 4) {
        printf("    mov [rax], r8\n");
    }
    if (depth == 5) {
        printf("    mov [rax], r9\n");
    }
    return;
    /* error2("function definition arguments should be identifier",0); */
}

void gen(Node *node) {
    switch (node->ty) {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case '=':
        if (node->lhs->ty == ND_DEREF) {
            gen(node->lhs->lhs);
        } else {
            gen_lval(node->lhs);
        }
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_E:
    case ND_NE:
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rax\n");
        printf("    pop rdi\n");
        printf("    cmp rdi, rax\n");
        if (node->ty == ND_E) {
            printf("    jne .L%d\n", jump_num);
        } else if (node->ty == ND_NE) {
            printf("    je .L%d\n", jump_num);
        } else {
            error2("either == or != should be used for condition", 0);
        }
        return;
    case ND_WHILE:
        printf(".Lbegin:\n");
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rax\n");
        printf("    jmp .Lbegin\n");
        printf(".L%d:\n", jump_num++);
        return;
    case ND_IF:
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rax\n");
        printf(".L%d:\n", jump_num++);
        return;
    case ND_FUNC:
        if (node->rhs != NULL) {
            printf("%s:\n", node->func_name);
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            printf("    sub rsp, %d\n",
                   locals->offset); // `offset` bytes allocated
            if (node->lhs != NULL) {
                gen_arg(node->lhs);
            }
            gen(node->rhs);
            return;
        }
        if (node->lhs != NULL) {
            gen(node->lhs);
            if (node->lhs->ty != ',') {
                printf("    pop rdi\n");
            }
        }
        printf("    call %s\n", node->func_name);
        printf("    push rax\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ',':
        gen(node->lhs);
        printf("    pop rax\n");
        switch (node->val) {
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
        if (node->rhs->ty == ND_NUM) {
            printf("    pop rax\n");
            switch (node->val) {
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
    case '{':
        gen(node->lhs);
        gen(node->rhs);
        return;
    case ND_ADDR:
        // TODO: can't process &(x+1)
        gen_lval(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
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
