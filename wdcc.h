#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== token ==================== */

typedef enum {
    TK_NUM = 256, // integer token
    TK_LVAR,      // variable token
    TK_EOF,       // token representing the end of input
    TK_IF,        // if statement
    TK_WHILE,     // while statement
    TK_E,         // equality
    TK_NE,        // nonequality
    TK_FUNC,
    TK_RETURN,
    TK_TYPE,
    TK_SIZEOF,
} TokenKind;

typedef struct {
    TokenKind ty;   // token type
    int val;        // if ty is TK_NUM or less, the value of it
    char name[100]; // if ty is TK_LVAR/TK_FUNC
    char *input;    // token character list for debugging
    int len;        // length of name
} Token;

extern Token tokens[100];

/* ==================== node ==================== */
typedef enum {
    // 0 to 255 is reserved for some of one character operations, like = or +
    ND_NUM = 256, // integer
    ND_LVAR,      // local variable
    ND_ADDR,      // & for &(local var)
    ND_DEREF,     // * for *(local var)
    ND_IF,        // if
    ND_E,         // ==
    ND_NE,        // !=
    ND_WHILE,     // while
    ND_FUNC,      // function
    ND_RETURN,    // return
} NodeKind;

typedef struct Type {
    enum { INT, PTR, ARRAY } ty;
    struct Type *ptr_to;
    size_t array_size;
} Type;

typedef struct LVar {
    // singly-linked list of local variables
    struct LVar *prev; // next local variable
    char *name;        // name of the local variable
    int len;           // length of the name
    int offset;        // offset from RBP
    Type *type;        // type of the variable
} LVar;

typedef struct Node {
    NodeKind ty;      // must be set to some value
    struct Node *lhs; // left-hand side
    struct Node *rhs; // right-hand side
    int val; // for ND_NUM or less. for argument of function, the depth of the
             // argument node
    char func_name[100]; // for ND_FUNC
    LVar *lvar;          // if ty==ND_LVAR,  pointer to corresponding lvar
} Node;

extern Node *code[100];

extern LVar *locals;

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

LVar *find_lvar_from_locals(Token *tok);

Node *assign();
Node *assign_prime();
Node *argument();
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
