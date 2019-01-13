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

extern Token tokens[100];

/* ==================== node ==================== */
enum {
    ND_NUM = 256, // type of integer/identifier node
    ND_IDENT,     // type of identifier node
};

typedef struct Node {
    int ty; // must be set to some value; operator symbol itself, ND_NUM=256 for
            // number or ND_IDENT for identifier
    struct Node *lhs; // left-hand side
    struct Node *rhs; // right-hand side
    int val;          // for number node
    char name;        // for identifier
} Node;

extern Node *code[100];

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

// prototype declaration
void tokenize(char *p);

Node *assign();
Node *assign_prime();
Node *add();
Node *mul();
Node *term();
void program();
Node *paragraph();
Node *paragraph_prime();

void gen(Node *node);
void error(int i);
void error2(char error_message[], int i);

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
void runtest();
