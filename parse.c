#include "wdcc.h"

Token tokens[100];

VarBlock *current_locals = NULL;

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

        if (strchr("&+-*/();={},[]", *p)) {
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

        if (!strncmp("sizeof", p, 6)) {
            tokens[i].ty = TK_SIZEOF;
            tokens[i].input = p;
            i++;
            p += 6;
            continue;
        }

        if (*p >= 'a' && *p <= 'z') {
            char *tmp;
            int name_len = 0;
            for (tmp = p; isalnum(*tmp); tmp++)
                // while following character is alnum...
                name_len++;
            if (name_len >= 100)
                error2("function/variable name longer than 100 character", i);
            if (*tmp == '(') {
                // if the word is followed by '(', it's a function
                tokens[i].ty = TK_FUNC;
            } else {
                tokens[i].ty = TK_LVAR;
            }
            tokens[i].input = p;
            strncpy(tokens[i].name, p, name_len);
            tokens[i].name[name_len] = 0; // EOF
            tokens[i].len = name_len;
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

VarBlock *find_lvar_from_locals(Token *tok) {
    for (VarBlock *var = current_locals; var; var = var->prev) {
        if (var->len == tok->len && !memcmp(tok->name, var->name, var->len))
            return var;
    }
    return NULL;
}

VarBlock *find_lvar_from_globals(Token *tok) {
    for (VarBlock *var = globals; var; var = var->prev) {
        if (var->len == tok->len && !memcmp(tok->name, var->name, var->len))
            return var;
    }
    return NULL;
}

Node *new_node_lvar(Token *tok, int declaration_type, int pointer_depth) {
    // if declaration_type is
    // 0: not declaration
    // 1: declaration
    // bigger than 1: declaration of an array, the value of `declaration_type`
    // being the length of the array
    Node *node = malloc(sizeof(Node));
    VarBlock *lvar = find_lvar_from_locals(tok);
    VarBlock *gvar = find_lvar_from_globals(tok);
    if (lvar) { // already declared
        node->var = lvar;
        node->ty = ND_LOC_VAR;
    } else if (gvar) { // already declared as global
        node->var = gvar;
        node->ty = ND_GLO_VAR;
    } else { // newly declared
        VarBlock *var;
        if (current_locals) {
            node->ty = ND_LOC_VAR;
            var = lvar;
            var = calloc(1, sizeof(VarBlock));
            var->prev = current_locals; // link
        } else {
            node->ty = ND_GLO_VAR;
            var = gvar;
            var = calloc(1, sizeof(VarBlock));
            var->prev = globals; // link
        }
        var->name = tok->name;
        var->len = tok->len;
        node->var = var;
        if (declaration_type > 1) { // array to int
            var->type = malloc(sizeof(Type));
            var->type->ty = ARRAY;
            Type *p = malloc(sizeof(Type));
            var->type->ptr_to = p;
            p->ty = INT;
            var->type->array_size = declaration_type;
            var->offset = current_locals->offset + 8 * declaration_type;
        } else if (pointer_depth != 0) { // pointer
            var->type = malloc(sizeof(Type));
            var->type->ty = PTR;
            Type *p = malloc(sizeof(Type));
            var->type->ptr_to = p;
            var->offset = current_locals->offset + 8;
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
                pbefore = p;
            }
        } else { // int
            var->type = malloc(sizeof(Type));
            var->type->ty = INT;
            var->offset = current_locals->offset + 8;
        }
        if (current_locals) {
            current_locals = var;
        } else {
            globals = var;
        }
    }
    return node;
}

Node *new_node_func(char name[], Node *lhs, Node *rhs, VarBlock *locals) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_FUNC;
    node->lhs = lhs;
    node->rhs = rhs;
    node->var = locals;
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
        if (!(lhs->ty == ND_LOC_VAR || lhs->ty == ND_GLO_VAR ||
              lhs->ty == ND_DEREF)) {
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
        VarBlock *locals = malloc(sizeof(VarBlock));
        VarBlock *current_locals_bak = malloc(sizeof(VarBlock));
        if (tokens[pos + 2].ty == ')') {
            pos += 3;
            lhs = NULL;
        } else {
            pos += 2;
            current_locals_bak = current_locals;
            current_locals = locals;
            lhs = argument();
            locals = current_locals;
            current_locals = current_locals_bak;
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
            return new_node_func(func_name, lhs, NULL, locals);
        } else {
            current_locals_bak = current_locals;
            current_locals = locals;
            Node *rhs = paragraph();
            locals = current_locals;
            current_locals = current_locals_bak;
            return new_node_func(func_name, lhs, rhs, locals);
        }
    }
    if (tokens[pos].ty == TK_NUM) {
        return new_node_num(tokens[pos++].val);
    }
    if (tokens[pos].ty == TK_LVAR) {
        if (tokens[pos + 1].ty == '[') {
            Node *base = new_node_lvar(
                &tokens[pos], 0,
                1); // TODO(optional): implement to allow 3[a] notation
            pos += 2;
            Node *offset = term();
            pos += 1;
            return new_node(ND_DEREF, new_node('+', base, offset), NULL);
        } else {
            return new_node_lvar(&tokens[pos++], 0, 0);
        }
    }
    if (tokens[pos].ty == TK_TYPE) {
        if (tokens[pos + 1].ty == TK_FUNC) {
            pos++;
            return term();
        }
        if (tokens[pos + 1].ty == TK_LVAR) {
            pos++;
            if (tokens[pos + 1].ty ==
                '[') { // array definition; only accepts number immediate
                Node *lhs = new_node_lvar(&tokens[pos], tokens[pos + 2].val,
                                          1); // variable is pointer to int
                pos += 4;
                return new_node(ND_DEF, lhs, NULL);
            } else {
                Node *lhs = new_node_lvar(&tokens[pos++], 1, 0);
                return new_node(ND_DEF, lhs, NULL);
            }
        }
        if (tokens[pos + 1].ty == '*') {
            int j = 0;
            while (tokens[++pos].ty == '*') {
                j++;
            }
            Node *lhs = new_node_lvar(&tokens[pos++], 1, j);
            return new_node(ND_DEF, lhs, NULL);
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
    if (tokens[pos].ty == TK_SIZEOF) {
        pos++;
        Node *node = term();
        if (node->ty == ND_NUM) { // node is number
            return new_node_num(4);
        } else if (node->var->type->ty == INT) { // node is int variable
            return new_node_num(4);
        } else if (node->var->type->ty == PTR) { // node is pointer variable
            return new_node_num(8);
        } else if (node->var->type->ty == ARRAY) { // node is array variable
            return new_node_num(4 * node->var->type->array_size);
        } else {
            error2("The argument to sizeof is uninterpretable:%s", pos);
        }
    }
    error2("The token is uninterpretable:%s", pos);
}
