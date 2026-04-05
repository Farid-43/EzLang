#ifndef RUNTIME_H
#define RUNTIME_H

#include "ast.h"

/* ---- Value types ---- */
#define VAL_NUM 0
#define VAL_DEC 1
#define VAL_CHAR 2
#define VAL_STR 3

// Runtime Value
typedef struct
{
    int type;
    double dval;
    char cval;
    char *sval;
} Val;

/* Opaque enough for parser actions (duplicate check + add). */
// Function Structure
typedef struct
{
    char name[64];
    int return_type;
    Node *params;
    Node *body;
} Func;

Func *findFunc(const char *name);
Func *addFunc(const char *name, int return_type, Node *params, Node *body);

void execBlock(Node *n);

#endif
