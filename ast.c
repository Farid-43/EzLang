#include <stdlib.h>
#include <string.h>

#include "ast.h"

Node *makeNode(int type)
{
    Node *n = (Node *)calloc(1, sizeof(Node));
    n->type = type;
    return n;
}

Node *makeNum(double d)
{
    Node *n = makeNode(N_NUM);
    n->dval = d;
    return n;
}

Node *makeDec(double d)
{
    Node *n = makeNode(N_DEC);
    n->dval = d;
    return n;
}

Node *makeChar(char c)
{
    Node *n = makeNode(N_CHAR);
    n->cval = c;
    return n;
}

Node *makeStr(char *s)
{
    Node *n = makeNode(N_STR);
    n->sval = strdup(s);
    return n;
}

Node *makeId(char *name)
{
    Node *n = makeNode(N_ID);
    n->sval = strdup(name);
    return n;
}

Node *makeBinop(char *op, Node *l, Node *r)
{
    Node *n = makeNode(N_BINOP);
    n->op = strdup(op);
    n->left = l;
    n->right = r;
    return n;
}

Node *makeUnop(char *op, Node *operand)
{
    Node *n = makeNode(N_UNOP);
    n->op = strdup(op);
    n->left = operand;
    return n;
}

Node *append(Node *list, Node *item)
{
    Node *p;

    if (!list)
        return item;

    p = list;
    while (p->next)
        p = p->next;
    p->next = item;

    return list;
}
