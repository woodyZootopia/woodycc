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

        if (!strncmp("!=", p, 2)) {
            tokens[i].ty = TK_NE;
            tokens[i].input = p;
            i++;
            p += 2;
            continue;
        }

        if (!strncmp("==", p, 2)) {
            tokens[i].ty = TK_E;
            tokens[i].input = p;
            i++;
            p += 2;
            continue;
        }

        if (strchr("&+-*/();={},", *p)) {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        if (!strncmp("if", p, 2)) {
            tokens[i].ty = TK_IF;
            tokens[i].input = p;
            i++;
            p += 2;
            continue;
        }

        if (!strncmp("while", p, 5)) {
            tokens[i].ty = TK_WHILE;
            tokens[i].input = p;
            i++;
            p += 5;
            continue;
        }

        if (!strncmp("return", p, 6)) {
            tokens[i].ty = TK_RETURN;
            tokens[i].input = p;
            i++;
            p += 6;
            continue;
        }

        if (!strncmp("int", p, 3)) {
            tokens[i].ty = TK_TYPE;
            tokens[i].input = p;
            i++;
            p += 3;
            continue;
        }

        if (*p >= 'a' && *p <= 'z') {
            char *tmp;
            int j = 0;
            for (tmp = p; isalnum(*tmp) || *tmp == ' '; tmp++)
                // while following character is alnum or space...
                j++;
            if (j >= 100)
                error2("function/variable name longer than 100 character", i);
            if (*tmp == '(') {
                // if the word is followed by '(', it's a function
                tokens[i].ty = TK_FUNC;
            } else {
                tokens[i].ty = TK_LVAR;
            }
            tokens[i].input = p;
            strncpy(tokens[i].name, p, j);
            tokens[i].name[j] = 0; // EOF
            tokens[i].len = j;
            p = tmp;
            i++;
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

Node *new_node(NodeKind ty, Node *lhs, Node *rhs) {
    // for nodes which is NOT number, function, local variable i.e. "if"
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

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->name, var->name, var->len))
            return var;
    }
    return NULL;
}

Node *new_node_lvar(Token *tok, int declaration, int pointer_depth) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_LVAR;
    // TODO: only expects one character variable
    LVar *lvar = find_lvar(tok);
    if (lvar) {
        node->offset = lvar->offset;
    } else {
        if (!declaration) {
            error2("The variable is not declared:%s", pos - 1);
        }
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->name;
        lvar->len = tok->len;
        if (pointer_depth != 0) {
            lvar->type = malloc(sizeof(Type));
            lvar->type->ty = PTR;
            Type *p = malloc(sizeof(Type));
            lvar->type->ptr_to = p;
            pointer_depth--;
            Type *pbefore = p;
            for (; pointer_depth >= 0; pointer_depth--) {
                Type *p = malloc(sizeof(Type));
                if (pointer_depth == 0) {
                    p->ty = INT;
                    break;
                }
                p->ty = PTR;
                p->ptr_to = pbefore;
            }
        } else {
            lvar->type = malloc(sizeof(Type));
            lvar->type->ty = INT;
        }
        lvar->offset = locals->offset + 8;
        node->offset = lvar->offset;
        locals = lvar;
    }
    return node;
}

Node *new_node_func(char name[], Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_FUNC;
    node->lhs = lhs;
    node->rhs = rhs;
    strcpy(node->func_name, name);
    return node;
}

void program() {
    int i = 0;
    while (tokens[pos].ty != TK_EOF) {
        code[i + 1] = NULL;
        code[i++] = paragraph();
    }
}

Node *paragraph() {
    if (tokens[pos].ty == TK_IF) {
        pos++;
        if (tokens[pos].ty != '(') {
            error2("if condition not stated", pos);
        }
        Node *lhs = assign();
        Node *rhs = paragraph();
        return new_node(ND_IF, lhs, rhs);
    }
    if (tokens[pos].ty == TK_WHILE) {
        pos++;
        if (tokens[pos].ty != '(') {
            error2("while condition not stated", pos);
        }
        Node *lhs = assign();
        Node *rhs = paragraph();
        return new_node(ND_WHILE, lhs, rhs);
    }
    if (tokens[pos].ty == '{') {
        pos++;
        Node *lhs = paragraph();
        if (tokens[pos].ty == '}') {
            pos++;
            return lhs;
        }
        Node *rhs = paragraph_prime();
        return new_node('{', lhs, rhs);
    }
    Node *lhs = assign();
    return lhs;
}

Node *paragraph_prime() {
    Node *lhs = paragraph();
    if (tokens[pos].ty == '}') {
        pos++;
        return lhs;
    }
    Node *rhs = paragraph_prime();
    return new_node('{', lhs, rhs);
}

Node *assign() {
    if (tokens[pos].ty == TK_RETURN) {
        pos++;
        Node *lhs = add();
        pos++;
        return new_node(ND_RETURN, lhs, NULL);
    }
    Node *lhs = add();
    if (tokens[pos].ty == ';') {
        pos++;
        return lhs;
    }
    if (tokens[pos].ty == '=') {
        if (!(lhs->ty == ND_LVAR || lhs->ty == ND_DEREF)) {
            error2("left hand side of assignment is not identifier:", pos);
        }
        pos++;
        Node *rhs = assign();
        return new_node('=', lhs, rhs);
    }
    if (tokens[pos].ty == TK_E) {
        pos++;
        Node *rhs = add();
        return new_node(ND_E, lhs, rhs);
    }
    if (tokens[pos].ty == TK_NE) {
        pos++;
        Node *rhs = add();
        return new_node(ND_NE, lhs, rhs);
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

Node *argument() {
    Node *lhs = add();
    if (tokens[pos].ty == ',') {
        pos++;
        Node *rhs = argument();
        return new_node(',', lhs, rhs);
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
    if (tokens[pos].ty == TK_FUNC) {
        if (tokens[pos + 1].ty != '(')
            error2("invalid function format.", pos);
        char *func_name = tokens[pos].name;
        Node *lhs;
        if (tokens[pos + 2].ty == ')') {
            pos += 3;
            lhs = NULL;
        } else {
            pos += 2;
            lhs = argument();
            pos++;
            int j = 0;
            // mark ',' with depth from function to determine the register to
            // mov
            for (Node *tmp = lhs; tmp->ty == ','; tmp = tmp->rhs) {
                tmp->val = j;
                j++;
            }
        }
        if (tokens[pos].ty != '{') {
            return new_node_func(func_name, lhs, NULL);
        }
        Node *rhs = paragraph();
        return new_node_func(func_name, lhs, rhs);
    }
    if (tokens[pos].ty == TK_NUM)
        return new_node_num(tokens[pos++].val);
    if (tokens[pos].ty == TK_LVAR)
        return new_node_lvar(&tokens[pos++], 0, 0);
    if (tokens[pos].ty == TK_TYPE) {
        if (tokens[pos + 1].ty == TK_FUNC) {
            pos++;
            return term();
        }
        if (tokens[pos + 1].ty == TK_LVAR) {
            pos++;
            return new_node_lvar(&tokens[pos++], 1, 0);
        }
        if (tokens[pos + 1].ty == '*') {
            int j = 0;
            while (tokens[++pos].ty == '*') {
                j++;
            }
            return new_node_lvar(&tokens[pos++], 1, j);
        }
        error2("int declaration is followed by something other than function "
               "or variable",
               pos);
    }
    if (tokens[pos].ty == '(') {
        pos++;
        Node *node = assign();
        if (tokens[pos].ty != ')')
            error2("No closing bracket corresponding to the opening:%s", pos);
        pos++;
        return node;
    }
    if (tokens[pos].ty == '&') {
        pos++;
        Node *lhs = term();
        return new_node(ND_ADDR, lhs, NULL);
    }
    if (tokens[pos].ty == '*') {
        pos++;
        Node *lhs = term();
        return new_node(ND_DEREF, lhs, NULL);
    }
    error2("The token is uninterpretable:%s", pos);
}
