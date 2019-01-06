#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== token ==================== */

enum {
    TK_NUM = 256, // integer token
    TK_IDENT,     // variable token
    TK_EOF,       // token representing the end of input
};

typedef struct {
    int ty;      // token type
    int val;     // if ty is TK_NUM/TK_IDENT, the value of it
    char *input; // token character list for debugging
} Token;

Token tokens[100];

void tokenize(char *p) {
    int i = 0;
    while (*p) {
        // skip space
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strchr("+-*/();=", *p)) {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        if (*p >= 'a' && *p <= 'z') {
            tokens[i].ty = TK_IDENT;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        fprintf(stderr, "Unable to tokenize:%s\n", p);
        exit(1);
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

void error(int i) {
    fprintf(stderr, "Unexpected token:%s\n", tokens[i].input);
    exit(1);
}

void error2(char error_message[], int i) {
    fprintf(stderr, error_message, tokens[i].input);
    exit(1);
}

/* ==================== node ==================== */

int pos = 0;

enum {
    ND_NUM = 256, // type of integer node
    ND_IDENT,     // type of identifier node
};

typedef struct Node {
    int ty; // must be set to some value; operator symbol itself or 256 for
            // number
    struct Node *lhs; // left-hand side
    struct Node *rhs; // right-hand side
    int val;          // for number node
    char name;        // for identifier
} Node;

Node *code[100];

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

// prototype declaration
Node *assign();
Node *assign_prime();
Node *add();
Node *mul();
Node *term();

void program() {
    int i = 0;
    while (tokens[pos].ty != TK_EOF) {
        code[i + 1] = NULL;
        code[i++] = assign();
    }
}

Node *assign() {
    Node *lhs = add();
    if (tokens[pos].ty == ';') {
        pos++;
        return lhs;
    }
    if (tokens[pos].ty == '=') {
        pos++;
        Node *rhs = assign();
        return (new_node('=', lhs, rhs));
    }
    return lhs;
}

Node *add() {
    Node *lhs = mul();
    if (tokens[pos].ty == '+') {
        pos++;
        return new_node('+', lhs, add());
    }
    if (tokens[pos].ty == '-') {
        pos++;
        return new_node('-', lhs, add());
    }
    return lhs;
}

Node *mul() {
    Node *lhs = term();
    if (tokens[pos].ty == '*') {
        pos++;
        return new_node('*', lhs, mul());
    }
    if (tokens[pos].ty == '/') {
        pos++;
        return new_node('/', lhs, mul());
    }
    return lhs;
}

Node *term() {
    if (tokens[pos].ty == TK_NUM)
        return new_node_num(tokens[pos++].val);
    if (tokens[pos].ty == TK_IDENT)
        return new_node_num(tokens[pos++].val);
    if (tokens[pos].ty == '(') {
        pos++;
        Node *node = add();
        if (tokens[pos].ty != ')')
            error2("No closing bracket corresponding to the opening:%s", pos);
        pos++;
        return node;
    }
    error2("The token is uninterpretable:%s", pos);
}

/* ==================== virtual stack machine ==================== */

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

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
    }
    printf("    push rax\n");
}

/* ==================== main ==================== */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect number of arguments.\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    tokenize(argv[1]);
    program();
    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
        printf("    pop rax\n");
    }

    printf("    ret\n");
    return 0;
}
