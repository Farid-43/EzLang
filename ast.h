#ifndef AST_H
#define AST_H

/* ---- AST node types ---- */
#define N_NUM 0
#define N_DEC 1
#define N_CHAR 2
#define N_STR 3
#define N_ID 4
#define N_BINOP 5
#define N_UNOP 6
#define N_SCAN 7
#define N_DECL 8
#define N_ASSIGN 9
#define N_SHOW 10
#define N_SEND 11
#define N_CHECK 12
#define N_DURING 13
#define N_ITERATE 14
#define N_CALL 15

/*
 * AST Node:
 *   left/right/extra = children
 *   next             = sibling link (statement lists, arg lists)
 */
typedef struct Node
{
    int type;
    double dval;
    char cval;
    char *sval;
    char *op;
    int vartype;
    int is_fixed;
    struct Node *left;
    struct Node *right;
    struct Node *extra;
    struct Node *next;
} Node;

// Function prototypes for AST construction
Node *makeNode(int type);
Node *makeNum(double d);
Node *makeDec(double d);
Node *makeChar(char c);
Node *makeStr(char *s);
Node *makeId(char *name);
Node *makeBinop(char *op, Node *l, Node *r);
Node *makeUnop(char *op, Node *operand);
Node *append(Node *list, Node *item);

#endif
