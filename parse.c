#include "wdcc.h"

Token tokens[100];

void error(int i) {
    fprintf(stderr, "Unexpected token:%s\n", tokens[i].input);
    exit(1);
}

void error2(char error_message[], int i) {
    fprintf(stderr, error_message, tokens[i].input);
    exit(1);
}

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
            tokens[i].val = (int)*p;
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

/* ==================== node ==================== */

int pos = 0;

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

Node *new_node_ident(char name) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = name;
    return node;
}

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
        return new_node_ident((char)tokens[pos++].val);
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
