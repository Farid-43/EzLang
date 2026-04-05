/*
 * codegen.c - C Code Generator for EzLang
 *
 * Generates equivalent C code from EzLang AST.
 * The generated code can be compiled with: gcc output.c -o output -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

/* ---- Function registry for code generation ---- */
#define MAX_FUNCS_CG 64

typedef struct
{
    char name[64];
    int return_type;
    Node *params;
    Node *body;
} FuncCG; // User function এর information store করে

static FuncCG func_registry[MAX_FUNCS_CG];
static int nfuncs_cg = 0;

/* ---- Internal prototypes ---- */
static void genExpr(FILE *out, Node *n);
static void genStmt(FILE *out, Node *n, int indent);
static void genBlock(FILE *out, Node *n, int indent);
static void genFunc(FILE *out, FuncCG *f);
static const char *typeToC(int vartype);
static void printIndent(FILE *out, int indent);

/* ===== Public Functions ===== */

void registerFuncForCodegen(const char *name, int return_type, Node *params, Node *body) // User-defined function register করে code generation এর জন্য
{
    if (nfuncs_cg >= MAX_FUNCS_CG)
        return;

    strncpy(func_registry[nfuncs_cg].name, name, 63);
    func_registry[nfuncs_cg].name[63] = '\0';
    func_registry[nfuncs_cg].return_type = return_type;
    func_registry[nfuncs_cg].params = params;
    func_registry[nfuncs_cg].body = body;
    nfuncs_cg++;
}

void generateCCode(FILE *out, Node *main_block)
{
    int i;

    /* Header */
    fprintf(out, "/*\n");
    fprintf(out, " * Generated C code from EzLang\n");
    fprintf(out, " * Compile with: gcc output.c -o output -lm\n");
    fprintf(out, " */\n\n");

    /* Includes */
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include <string.h>\n");
    fprintf(out, "#include <math.h>\n\n");

    /* Forward declarations for user functions
    - প্রতিটা user function এর forward declaration generate করে
    - Example: `int add(int a, int b);`
    */
    for (i = 0; i < nfuncs_cg; i++)
    {
        Node *p;
        fprintf(out, "%s %s(", typeToC(func_registry[i].return_type), func_registry[i].name);
        p = func_registry[i].params;
        while (p)
        {
            fprintf(out, "%s %s", typeToC(p->vartype), p->sval);
            p = p->next;
            if (p)
                fprintf(out, ", ");
        }
        fprintf(out, ");\n");
    }
    if (nfuncs_cg > 0)
        fprintf(out, "\n");

    /* User-defined functions
    - প্রতিটা user function এর full definition generate করে
    - `genFunc()` call করে যা function এর body generate করে
    */
    for (i = 0; i < nfuncs_cg; i++)
    {
        genFunc(out, &func_registry[i]);
        fprintf(out, "\n");
    }

    /* Main function */
    fprintf(out, "int main(void)\n");
    fprintf(out, "{\n");
    genBlock(out, main_block, 1);
    fprintf(out, "}\n");
}

/* ===== Helper Functions ===== */

static const char *typeToC(int vartype)
{
    switch (vartype)
    {
    case 0:
        return "int"; /* VAL_NUM */
    case 1:
        return "double"; /* VAL_DEC */
    case 2:
        return "char"; /* VAL_CHAR */
    case 3:
        return "char*"; /* VAL_STR */
    default:
        return "int";
    }
}

static void printIndent(FILE *out, int indent)
{
    int i;
    for (i = 0; i < indent; i++)
        fprintf(out, "    ");
}

/* ===== Expression Generator ===== */

static void genExpr(FILE *out, Node *n)
{
    if (!n)
        return;

    switch (n->type)
    {
    case N_NUM:
        fprintf(out, "%d", (int)n->dval);
        break;

    case N_DEC:
        fprintf(out, "%g", n->dval);
        break;

    case N_CHAR:
        fprintf(out, "'%c'", n->cval);
        break;

    case N_STR:
        fprintf(out, "\"%s\"", n->sval);
        break;

    case N_ID:
        fprintf(out, "%s", n->sval);
        break;

    case N_BINOP:
    {
        const char *op = n->op;

        /* Special cases for EzLang operators */
        if (!strcmp(op, "power"))
        {
            fprintf(out, "pow(");
            genExpr(out, n->left);
            fprintf(out, ", ");
            genExpr(out, n->right);
            fprintf(out, ")");
        }
        else if (!strcmp(op, "and"))
        {
            fprintf(out, "(");
            genExpr(out, n->left);
            fprintf(out, " && ");
            genExpr(out, n->right);
            fprintf(out, ")");
        }
        else if (!strcmp(op, "or"))
        {
            fprintf(out, "(");
            genExpr(out, n->left);
            fprintf(out, " || ");
            genExpr(out, n->right);
            fprintf(out, ")");
        }
        else
        {
            /* Standard operators: +, -, *, /, %, >, <, >=, <=, ==, != */
            fprintf(out, "(");
            genExpr(out, n->left);
            fprintf(out, " %s ", op);
            genExpr(out, n->right);
            fprintf(out, ")");
        }
        break;
    }

    case N_UNOP:
    {
        const char *op = n->op;

        if (!strcmp(op, "not"))
        {
            fprintf(out, "(!");
            genExpr(out, n->left);
            fprintf(out, ")");
        }
        else if (!strcmp(op, "neg"))
        {
            fprintf(out, "(-");
            genExpr(out, n->left);
            fprintf(out, ")");
        }
        else if (!strcmp(op, "root"))
        {
            fprintf(out, "sqrt(");
            genExpr(out, n->left);
            fprintf(out, ")");
        }
        break;
    }

    case N_CALL:
    {
        Node *arg;
        fprintf(out, "%s(", n->sval); // function name
        arg = n->left;
        while (arg) // arguments
        {
            genExpr(out, arg);
            arg = arg->next;
            if (arg)
                fprintf(out, ", ");
        }
        fprintf(out, ")");
        break;
    }

    case N_SCAN: // scan() function এর জন্য placeholder generate করে
        /* scan() becomes a scanf placeholder - simplified */
        fprintf(out, "0 /* scan() - use scanf */");
        break;

    default:
        fprintf(out, "/* unknown expr type %d */", n->type);
        break;
    }
}

/* ===== Statement Generator ===== */

static void genStmt(FILE *out, Node *n, int indent)
{
    if (!n)
        return;

    switch (n->type)
    {
    case N_DECL:
    {
        printIndent(out, indent);
        if (n->is_fixed)
            fprintf(out, "const ");
        fprintf(out, "%s %s", typeToC(n->vartype), n->sval);
        if (n->left)
        {
            fprintf(out, " = ");
            genExpr(out, n->left);
        }
        fprintf(out, ";\n");
        break;
    }
        /* Variable declaration এর জন্য C code generate করে
    | EzLang             | C Code              |
    | ------------------ | ------------------- |
    | `num x = 5`        | `int x = 5;`        |
    | `dec y = 3.14`     | `double y = 3.14;`  |
    | `fixed num PI = 3` | `const int PI = 3;` |
        */

    case N_ASSIGN:
    {
        printIndent(out, indent);
        fprintf(out, "%s = ", n->sval);
        genExpr(out, n->left);
        fprintf(out, ";\n");
        break;
    }

    case N_SHOW:
    {
        Node *arg = n->left;
        printIndent(out, indent);

        /* Generate printf with format string */
        fprintf(out, "printf(\"");

        /* Build format string */
        Node *a = arg;
        while (a)
        {
            switch (a->type)
            {
            case N_STR:
                fprintf(out, "%%s");
                break;
            case N_CHAR:
                fprintf(out, "%%c");
                break;
            case N_NUM:
                fprintf(out, "%%d");
                break;
            case N_DEC:
                fprintf(out, "%%g");
                break;
            case N_ID:
                /* We don't know the type, use generic %g for numbers */
                fprintf(out, "%%g");
                break;
            default:
                fprintf(out, "%%g");
                break;
            }
            a = a->next;
        }
        fprintf(out, "\\n\"");

        /* Arguments */
        a = arg;
        while (a)
        {
            fprintf(out, ", ");
            if (a->type == N_ID)
                fprintf(out, "(double)"); // ID এর জন্য generic cast to double করে (যেহেতু type information নেই)
            genExpr(out, a);
            a = a->next;
        }
        fprintf(out, ");\n");
        break;
    }

    case N_SEND:
    {
        printIndent(out, indent);
        fprintf(out, "return ");
        if (n->left)
            genExpr(out, n->left);
        else
            fprintf(out, "0");
        fprintf(out, ";\n");
        break;
    }
        /*
        - EzLang: `send 0` → C: `return 0;`
        - EzLang: `send x + 1` → C: `return (x + 1);`
        */

    case N_CHECK:
    {
        Node *c = n;
        int first = 1;

        while (c)
        {
            printIndent(out, indent);

            if (c->left == NULL)
            {
                /* else block */
                fprintf(out, "else\n"); // otherwise -> else block
            }
            else if (first)
            {
                /* if block */
                fprintf(out, "if (");
                genExpr(out, c->left);
                fprintf(out, ")\n");
                first = 0;
            }
            else
            {
                /* else if block */
                fprintf(out, "else if (");
                genExpr(out, c->left);
                fprintf(out, ")\n");
            }

            printIndent(out, indent);
            fprintf(out, "{\n");
            genBlock(out, c->right, indent + 1); /* body is in 'right' */
            printIndent(out, indent);
            fprintf(out, "}\n");

            c = c->extra; /* next branch is in 'extra' */
        }
        break;
    }

    case N_DURING:
    {
        printIndent(out, indent);
        fprintf(out, "while (");
        genExpr(out, n->left);
        fprintf(out, ")\n");
        printIndent(out, indent);
        fprintf(out, "{\n");
        genBlock(out, n->right, indent + 1); /* body is in 'right' */
        printIndent(out, indent);
        fprintf(out, "}\n");
        break;
    }

    case N_ITERATE:
    {
        printIndent(out, indent);
        fprintf(out, "for (int %s = ", n->sval);
        genExpr(out, n->left);
        fprintf(out, "; %s <= ", n->sval);
        genExpr(out, n->right);
        fprintf(out, "; %s++)\n", n->sval);
        printIndent(out, indent);
        fprintf(out, "{\n");
        genBlock(out, n->extra, indent + 1); /* body is in 'extra' */
        printIndent(out, indent);
        fprintf(out, "}\n");
        break;
    }

    default:
        printIndent(out, indent);
        fprintf(out, "/* unknown statement type %d */\n", n->type);
        break;
    }
}

/* ===== Block Generator ===== */

static void genBlock(FILE *out, Node *n, int indent)
{
    while (n)
    {
        genStmt(out, n, indent);
        n = n->next;
    }
}

/* ===== Function Generator ===== */

static void genFunc(FILE *out, FuncCG *f)
{
    Node *p;

    fprintf(out, "%s %s(", typeToC(f->return_type), f->name);

    p = f->params;
    while (p)
    {
        fprintf(out, "%s %s", typeToC(p->vartype), p->sval);
        p = p->next;
        if (p)
            fprintf(out, ", ");
    }
    fprintf(out, ")\n{\n");

    genBlock(out, f->body, 1);

    fprintf(out, "}\n");
}
/*
EzLang:                          C Code:
num add(num a, num b) begin      int add(int a, int b)
    send a + b                   {
end                                  return (a + b);
                                 }
*/
/*
| Function                   Purpose                          |
| -------------------------- -------------------------------- |
| `registerFuncForCodegen()` User function register করে       |
| `generateCCode()`          Full C program generate করে      |
| `typeToC()`                EzLang type → C type             |
| `printIndent()`            Indentation print করে            |
| `genExpr()`                Expression → C code              |
| `genStmt()`                Statement → C code               |
| `genBlock()`               Block of statements generate করে |
| `genFunc()`                Function definition generate করে |
*/