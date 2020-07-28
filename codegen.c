#include "wdcc.h"

#define PRINT_COMMENT_DEBUG

// TODO: align 16 bits for ABI proportionate call
int jump_num = 2;

void gen_lval(Node *node) {
    if (!(node->ty == ND_LOC_VAR || node->ty == ND_DEF)) {
        error2("left hand side is not a variable", 0);
    }

    int offset;
    if (node->ty == ND_LOC_VAR) {
        offset = node->var->offset;
    } else if (node->ty == ND_DEF) {
        offset = node->lhs->var->offset;
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n"); // now rax is the pointer to the variable
}

void gen_arg(Node *node) {
    int depth = 0;
    Node *tmp;
    if (node->ty == ND_DEF) {
        gen_lval(node->lhs);
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        return;
    }
    for (tmp = node; tmp->ty != ND_DEF; tmp = tmp->rhs) {
        gen_lval(tmp->lhs->lhs);
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
    gen_lval(tmp->lhs); // tmp is already the last argument
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
#ifdef PRINT_COMMENT_DEBUG
        printf("# push immediate at %d\n", __LINE__);
#endif
        printf("    push %d\n", node->val);
        return;
    case ND_LOC_VAR:
#ifdef PRINT_COMMENT_DEBUG
        printf("# generate local lhs at %d\n", __LINE__);
#endif
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_GLO_VAR:
#ifdef PRINT_COMMENT_DEBUG
        printf("# generate global lhs at %d\n", __LINE__);
#endif
        printf("%s:", node->var->name);
        printf("    .zero %d\n", 4 * node->var->len);
        // TODO: implement this; needs the info of array length
        return;
    case '=':
#ifdef PRINT_COMMENT_DEBUG
        printf("# assignment at %d\n", __LINE__);
#endif
        if (node->lhs->ty == ND_DEREF) {
#ifdef PRINT_COMMENT_DEBUG
            printf("# generate lhs (pointer dereference at %d)\n", __LINE__);
#endif
            gen(node->lhs->lhs);
        } else if (node->lhs->ty == ND_LOC_VAR) {
#ifdef PRINT_COMMENT_DEBUG
            printf("# generate lhs at %d\n", __LINE__);
#endif
            gen_lval(node->lhs);
        } else if (node->lhs->ty == ND_GLO_VAR) {
#ifdef PRINT_COMMENT_DEBUG
            printf("# generate lhs (global) at %d\n");
#endif
            printf("    mov %s, rax\n", node->lhs->var->name);
            printf("    push rax\n");
        } else {
            error2("lhs of the assignment is not appropriate.", 0);
        }
        gen(node->rhs);

#ifdef PRINT_COMMENT_DEBUG
        printf("# assign at %d\n", __LINE__);
#endif
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_E:
    case ND_NE:
#ifdef PRINT_COMMENT_DEBUG
        printf("# equality/nonequality at %d\n", __LINE__);
#endif
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
#ifdef PRINT_COMMENT_DEBUG
        printf("# while at %d\n", __LINE__);
#endif
        printf(".Lbegin:\n");
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rax\n");
        printf("    jmp .Lbegin\n");
        printf(".L%d:\n", jump_num++);
        return;
    case ND_IF:
#ifdef PRINT_COMMENT_DEBUG
        printf("# if at %d\n", __LINE__);
#endif
        gen(node->lhs);
        gen(node->rhs);
        printf("    pop rax\n");
        printf(".L%d:\n", jump_num++);
        return;
    case ND_FUNC:
#ifdef PRINT_COMMENT_DEBUG
        printf("# function at %d\n", __LINE__);
#endif
        if (node->rhs != NULL) { // function definition
            printf("%s:\n", node->func_name);
            printf("    push rbp\n");
            printf("    mov rbp, rsp\n");
            if (node->var) {
                printf("    sub rsp, %d\n",
                       node->var->offset); // allocate `offset` bytes
            }
            if (node->lhs != NULL) {
#ifdef PRINT_COMMENT_DEBUG
                printf("# argument assignment at %d\n", __LINE__);
#endif
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
#ifdef PRINT_COMMENT_DEBUG
        printf("# return at %d\n", __LINE__);
#endif
        gen(node->lhs);
#ifdef PRINT_COMMENT_DEBUG
        printf("# return at %d\n", __LINE__);
#endif
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
        if (node->rhs->ty != ',') {
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
    case ND_ADDR: // '&'
        // TODO: can't process &(x+1)
        gen_lval(node->lhs);
        return;
    case ND_DEREF: // '*'
#ifdef PRINT_COMMENT_DEBUG
        printf("# deref at %d\n", __LINE__);
#endif
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_DEF:
        if (node->lhs->ty == ND_GLO_VAR) {
#ifdef PRINT_COMMENT_DEBUG
            printf("# generate global lhs at %d\n", __LINE__);
#endif
            printf("%s:", node->var->name);
            printf("    .zero %d\n", 4 * node->var->len);
            // TODO: implement this; needs the info of array length
            return;
        } else if (node->lhs->ty == ND_LOC_VAR) {
            return;
        }
    }

    // If none of the above, the node is polynomial

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    if (node->ty == '+' || node->ty == '-') {
        if (node->lhs->var != NULL &&
            node->lhs->var->type->ty == PTR) { // if lhs is pointer
            if (node->lhs->var->type->ptr_to->ty ==
                INT) { // if lhs is pointer to int
                printf("    imul rdi, 4\n");
            } else { // if lhs is pointer to pointer
                printf("    imul rdi, 8\n");
            }
        }
    }

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
    default:
        error2("operator not known\n", 0);
    }
    printf("    push rax\n");
}
