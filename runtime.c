#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "runtime.h"

/* ---- Symbol table ---- */
#define MAX_SYMS 256

typedef struct
{
    char name[64];
    int vartype;
    int is_fixed;
    Val val;
} Sym;

static Sym symtab[MAX_SYMS];
static int nsyms = 0;

/* ---- Function table ---- */
#define MAX_FUNCS 64

static Func functab[MAX_FUNCS];
static int nfuncs = 0;

/* ---- Runtime state ---- */
static int send_flag = 0;
static int in_function = 0;
static int function_returned = 0;
static Val function_return_value;

extern FILE *yyout;

/* ---- Internal prototypes  ---- */
static Sym *findSym(const char *name);
static Sym *addSym(const char *name, int vartype, int is_fixed);
static Val evalNode(Node *n);
static Val coerce(Val v, int vartype);
static void printVal(Val v);
static void execNode(Node *n);

/* ===== Function table ===== */

Func *findFunc(const char *name)
{
    int i;
    for (i = 0; i < nfuncs; i++)
        if (strcmp(functab[i].name, name) == 0)
            return &functab[i];
    return NULL;
}

Func *addFunc(const char *name, int return_type, Node *params, Node *body)
{
    Func *f;
    if (nfuncs >= MAX_FUNCS)
    {
        fprintf(stderr, "Function table full\n");
        exit(1);
    }
    f = &functab[nfuncs++]; // Next available slot
    strncpy(f->name, name, 63);
    f->name[63] = '\0';
    f->return_type = return_type;
    f->params = params;
    f->body = body;
    return f;
}

/* ===== Symbol table ===== */

static Sym *findSym(const char *name) // Look up a symbol by name (Variable lookup)
{
    int i;
    for (i = 0; i < nsyms; i++)
        if (strcmp(symtab[i].name, name) == 0)
            return &symtab[i];
    return NULL;
}

static Sym *addSym(const char *name, int vartype, int is_fixed) // Add a new symbol to the table (Variable declaration)
{
    Sym *s;
    if (nsyms >= MAX_SYMS)
    {
        fprintf(stderr, "Symbol table full\n");
        exit(1);
    }
    s = &symtab[nsyms++]; // Next available slot
    strncpy(s->name, name, 63);
    s->name[63] = '\0';
    s->vartype = vartype;
    s->is_fixed = is_fixed;
    s->val.type = vartype;
    s->val.dval = 0;
    s->val.cval = 0;
    s->val.sval = NULL;
    return s;
}

/* ===== Implicit type coercion ===== */

static Val coerce(Val v, int vartype)
{
    if (vartype == VAL_NUM && v.type != VAL_STR) // decimal to number (3.14 -> 3)
    {
        v.type = VAL_NUM;
        v.dval = (double)(int)v.dval;
    }
    else if (vartype == VAL_CHAR && v.type != VAL_STR) // number to char (65 -> 'A')
    {
        v.type = VAL_CHAR;
        v.cval = (char)(int)v.dval;
        v.dval = (double)v.cval;
    }
    return v;
}

/* ===== Print a runtime value ===== */

static void printVal(Val v)
{
    switch (v.type)
    {
    case VAL_NUM:
        fprintf(yyout, "%d", (int)v.dval);
        break;
    case VAL_DEC:
        fprintf(yyout, "%g", v.dval); // remove trailing zeros for decimals
        break;
    case VAL_CHAR:
        fprintf(yyout, "%c", v.cval);
        break;
    case VAL_STR:
        fprintf(yyout, "%s", v.sval ? v.sval : "");
        break;
    }
}

/* ===== Expression evaluator ===== */

static Val evalNode(Node *n)
{
    Val v, l, r, a;
    v.type = VAL_NUM;
    v.dval = 0;
    v.cval = 0;
    v.sval = NULL;
    if (!n)
        return v;

    switch (n->type)
    {
    case N_NUM:
        v.type = VAL_NUM;
        v.dval = n->dval;
        break;
    case N_DEC:
        v.type = VAL_DEC;
        v.dval = n->dval;
        break;
    case N_CHAR:
        v.type = VAL_CHAR;
        v.cval = n->cval;
        v.dval = (double)n->cval; // Store char as double for easier arithmetic
        break;
    case N_STR:
        v.type = VAL_STR;
        v.sval = n->sval;
        break;

    case N_ID: // Symbol table থেকে variable খুঁজে তার current value return করে।
    {
        Sym *s = findSym(n->sval);
        if (!s)
        {
            fprintf(stderr, "Error: undefined variable '%s'\n", n->sval);
            break;
        }
        v = s->val;
        break;
    }

    case N_BINOP:
    {
        l = evalNode(n->left);
        r = evalNode(n->right);
        if (!strcmp(n->op, "+"))
        {
            v.type = (l.type == VAL_DEC || r.type == VAL_DEC) ? VAL_DEC : VAL_NUM;
            v.dval = l.dval + r.dval;
        }
        else if (!strcmp(n->op, "-"))
        {
            v.type = (l.type == VAL_DEC || r.type == VAL_DEC) ? VAL_DEC : VAL_NUM;
            v.dval = l.dval - r.dval;
        }
        else if (!strcmp(n->op, "*"))
        {
            v.type = (l.type == VAL_DEC || r.type == VAL_DEC) ? VAL_DEC : VAL_NUM;
            v.dval = l.dval * r.dval;
        }
        else if (!strcmp(n->op, "/"))
        {
            v.type = VAL_DEC;
            v.dval = (r.dval != 0) ? l.dval / r.dval : 0;
        }
        else if (!strcmp(n->op, "%"))
        {
            v.type = VAL_NUM;
            v.dval = (double)((int)l.dval % (int)r.dval);
        }
        else if (!strcmp(n->op, "power"))
        {
            v.type = VAL_DEC;
            v.dval = pow(l.dval, r.dval);
        }
        else if (!strcmp(n->op, ">"))
        {
            v.type = VAL_NUM;
            v.dval = l.dval > r.dval ? 1 : 0;
        }
        else if (!strcmp(n->op, "<"))
        {
            v.type = VAL_NUM;
            v.dval = l.dval < r.dval ? 1 : 0;
        }
        else if (!strcmp(n->op, ">="))
        {
            v.type = VAL_NUM;
            v.dval = l.dval >= r.dval ? 1 : 0;
        }
        else if (!strcmp(n->op, "<="))
        {
            v.type = VAL_NUM;
            v.dval = l.dval <= r.dval ? 1 : 0;
        }
        else if (!strcmp(n->op, "=="))
        {
            v.type = VAL_NUM;
            if (l.type == VAL_STR && r.type == VAL_STR)
                v.dval = strcmp(l.sval, r.sval) == 0 ? 1 : 0;
            else if (l.type == VAL_CHAR && r.type == VAL_CHAR)
                v.dval = l.cval == r.cval ? 1 : 0;
            else
                v.dval = l.dval == r.dval ? 1 : 0;
        }
        else if (!strcmp(n->op, "!="))
        {
            v.type = VAL_NUM;
            if (l.type == VAL_STR && r.type == VAL_STR)
                v.dval = strcmp(l.sval, r.sval) != 0 ? 1 : 0;
            else
                v.dval = l.dval != r.dval ? 1 : 0;
        }
        else if (!strcmp(n->op, "and"))
        {
            v.type = VAL_NUM;
            v.dval = (l.dval && r.dval) ? 1 : 0;
        }
        else if (!strcmp(n->op, "or"))
        {
            v.type = VAL_NUM;
            v.dval = (l.dval || r.dval) ? 1 : 0;
        }
        break;
    }

    case N_UNOP:
    {
        a = evalNode(n->left);
        if (!strcmp(n->op, "not"))
        {
            v.type = VAL_NUM;
            v.dval = a.dval ? 0 : 1;
        }
        else if (!strcmp(n->op, "neg"))
        {
            v = a;
            v.dval = -a.dval;
        }
        else if (!strcmp(n->op, "root"))
        {
            v.type = VAL_DEC;
            v.dval = sqrt(a.dval);
        }
        break;
    }

    case N_SCAN: // user input নিয়ে সেটা number হলে number হিসেবে, না হলে string হিসেবে treat করে return করে।
    {
        char buf[256];
        char *endp;
        double d;
        fprintf(yyout, "? ");
        fflush(yyout);
        if (fgets(buf, sizeof(buf), stdin))
        {
            buf[strcspn(buf, "\n")] = '\0'; // newline remove
            d = strtod(buf, &endp);         // Try to parse as number first
            if (endp != buf && *endp == '\0')
            {
                // parsed as number successfully
                v.type = (strchr(buf, '.') != NULL) ? VAL_DEC : VAL_NUM;
                v.dval = d;
            }
            else
            {
                // not a number, treat as string
                v.type = VAL_STR;
                v.sval = strdup(buf);
            }
        }
        break;
    }

    case N_CALL:
    {
        Func *f;
        Sym saved_symtab[MAX_SYMS];
        Val args[64];
        Val result;
        Node *anode;
        Node *pnode;
        int argc = 0;
        int pcount = 0;
        int i;
        int saved_nsyms;
        int saved_send_flag;
        int saved_in_function;
        int saved_function_returned;
        Val saved_function_return_value;

        result.type = VAL_NUM;
        result.dval = 0;
        result.cval = 0;
        result.sval = NULL;

        f = findFunc(n->sval); // find function by name
        if (!f)
        {
            fprintf(stderr, "Error: undefined function '%s'\n", n->sval);
            v = result;
            break;
        }

        if (in_function) // check recursive call (not supported)
        {
            fprintf(stderr, "Error: nested/recursive user function calls are not supported ('%s')\n", n->sval);
            v = result;
            break;
        }

        // Evaluate arguments
        anode = n->left;
        while (anode)
        {
            if (argc >= 64)
            {
                fprintf(stderr, "Error: too many arguments in function call '%s'\n", n->sval);
                v = result;
                break;
            }
            args[argc++] = evalNode(anode);
            anode = anode->next;
        }
        if (argc >= 64)
            break;

        pnode = f->params;
        while (pnode)
        {
            pcount++;
            pnode = pnode->next;
        }
        // Check argument count
        if (argc != pcount)
        {
            fprintf(stderr, "Error: function '%s' expects %d args, got %d\n", n->sval, pcount, argc);
            v = result;
            break;
        }
        // save current runtime state
        memcpy(saved_symtab, symtab, sizeof(symtab));
        saved_nsyms = nsyms;
        saved_send_flag = send_flag;
        saved_in_function = in_function;
        saved_function_returned = function_returned;
        saved_function_return_value = function_return_value;

        in_function = 1;
        function_returned = 0;
        send_flag = 0;
        function_return_value = result;

        // Bind parameters to arguments
        pnode = f->params;
        for (i = 0; i < argc; i++)
        {
            Sym *ps = addSym(pnode->sval, pnode->vartype, 0);
            ps->val = coerce(args[i], pnode->vartype);
            pnode = pnode->next;
        }

        // Execute function body
        execBlock(f->body);

        if (function_returned)
        {
            result = coerce(function_return_value, f->return_type);
        }
        else
        {
            result.type = f->return_type;
            result.dval = 0;
            result.cval = 0;
            result.sval = NULL;
        }

        // restore runtime state
        memcpy(symtab, saved_symtab, sizeof(symtab));
        nsyms = saved_nsyms;
        send_flag = saved_send_flag;
        in_function = saved_in_function;
        function_returned = saved_function_returned;
        function_return_value = saved_function_return_value;

        v = result;
        break;
    }

    default:
        break;
    }
    return v;
}
/*
┌─────────────────────────────────────────────────────┐
│  add(2, 3) call                                     │
├─────────────────────────────────────────────────────┤
│  1. Find "add" in functab                           │
│  2. Evaluate args: [2, 3]                           │
│  3. Save current symtab (backup)                    │
│  4. Create params: a=2, b=3                         │
│  5. Execute body: send a + b                        │
│  6. function_return_value = 5                       │
│  7. Restore old symtab                              │
│  8. Return Val{type:VAL_NUM, dval:5}                │
└─────────────────────────────────────────────────────┘

*/

/* ===== Statement executor ===== */

void execBlock(Node *n) // linked list of statements (Node *n) নেয় এবং প্রতিটা statement execute করে যতক্ষণ না send() call হয় বা function return হয়।
{
    while (n && !send_flag && !function_returned)
    {
        execNode(n);
        n = n->next;
    }
}

static void execNode(Node *n)
{
    if (!n || send_flag)
        return;

    switch (n->type)
    {

    case N_DECL: // variable declaration (with optional initialization)
    {
        Sym *s = findSym(n->sval);
        if (s) // variable already exists, check if it's a redeclaration or assignment to existing variable
        {
            if (n->left)
                s->val = coerce(evalNode(n->left), s->vartype);
            break;
        }
        // variable doesn't exist, add new symbol
        s = addSym(n->sval, n->vartype, n->is_fixed);
        if (n->left)
            s->val = coerce(evalNode(n->left), n->vartype);
        break;
    }

    case N_ASSIGN: // variable assignment
    {
        Sym *s = findSym(n->sval);
        if (!s) // variable must exist to assign
        {
            fprintf(stderr, "Error: '%s' not declared\n", n->sval);
            break;
        }
        if (s->is_fixed)
        {
            fprintf(stderr, "Error: cannot assign to constant '%s'\n", n->sval);
            break;
        }
        s->val = coerce(evalNode(n->left), s->vartype);
        break;
    }

    case N_SHOW: // print statement, argument হিসেবে যেকোন expression নিতে পারে এবং সেটা evaluate করে output দেয়।
    {
        Node *arg = n->left;
        while (arg)
        {
            printVal(evalNode(arg));
            arg = arg->next;
        }
        fprintf(yyout, "\n");
        break;
    }

    case N_SEND: // return statement
    {
        if (in_function)
        {
            // in function, set return value and flag
            Val rv;
            rv.type = VAL_NUM;
            rv.dval = 0;
            rv.cval = 0;
            rv.sval = NULL;
            if (n->left)
                rv = evalNode(n->left);
            function_return_value = rv;
            function_returned = 1;
        }
        else
        {
            // in main, stop program
            send_flag = 1;
        }
        break;
        /*
        - Main block এ → program stop
        - Function এ → return value set
        */
    }

    case N_CHECK:
    {
        Node *branch = n;
        while (branch)
        {
            if (!branch->left) // else block (no condition)
            {
                execBlock(branch->right);
                break;
            }
            if (evalNode(branch->left).dval) // condition true
            {
                execBlock(branch->right);
                break;
            }
            branch = branch->extra; // next branch
        }
        break;
        // check-else if-else structure handle করে, যেখানে left এ condition থাকে আর right এ body থাকে। extra pointer দিয়ে next branch (else if বা else) এ যায়।
    }

    case N_DURING: // while loop
    {
        while (!send_flag && evalNode(n->left).dval)
            execBlock(n->right);
        break;
        // while loop execute করে যতক্ষণ না condition false হয় বা send() call হয়। left এ condition আর right এ body থাকে।
    }

    case N_ITERATE: // for loop
    {
        Val start, limit;
        Sym *s = findSym(n->sval); // loop variable name n->sval থেকে symbol table এ খুঁজে নেয়। যদি না থাকে, নতুন variable হিসেবে add করে নেয়।
        if (!s)
            s = addSym(n->sval, VAL_NUM, 0);
        start = evalNode(n->left); // loop variable এর initial value n->left থেকে evaluate করে নেয়।
        limit = evalNode(n->right);
        s->val = coerce(start, VAL_NUM);                // loop variable কে number type এ coerce করে নেয় যাতে arithmetic operations সহজ হয়।
        while (!send_flag && s->val.dval <= limit.dval) // loop variable এর value limit এর value এর থেকে ছোট বা সমান থাকলে loop চালায়। প্রতি iteration এ loop variable এর value 1 বাড়ায়।
        {
            execBlock(n->extra);
            s->val.dval += 1.0;
        }
        break;
    }

    default:
        break;
    }
}

/*
| Section        | Functions               | কাজ                       |
| -------------- | ----------------------- | -------------------------- |
| Function Table | `findFunc`, `addFunc`   | User function store/lookup | // user-defined function গুলো store করে রাখে এবং lookup করে দেয়।
| Symbol Table   | `findSym`, `addSym`     | Variable store/lookup      | // variables গুলো store করে রাখে এবং lookup করে দেয়।
| Type System    | `coerce`                | Implicit type conversion   | // implicit type coercion handle করে, যেমন number থেকে char বা decimal থেকে number ইত্যাদি।
| Output         | `printVal`              | Value print                | // runtime values print করে দেয়।
| Evaluator      | `evalNode`              | Expression → Value         | // expression node নেয় এবং সেটা evaluate করে Val return করে।
| Executor       | `execBlock`, `execNode` | Statement execution        | // statement list নিয়ে কাজ করে, statement type অনুযায়ী appropriate action নেয়।

**Execution Flow:**

Source Code
    ↓ (Flex/Bison)
AST (Node tree)
    ↓ (execBlock) // main block বা function body এর AST node নেয় এবং প্রতিটা statement execute করে যতক্ষণ না send() call হয় বা function return হয়।
Statement by statement  // প্রতিটা statement এর type অনুযায়ী execNode() call হয়, যা statement execute করে।
    ↓ (execNode)
Each statement type // variable declaration, assignment, print, function call, if-else, loops ইত্যাদি handle করে।
    ↓ (evalNode for expressions) // expression node নেয় এবং সেটা evaluate করে Val return করে।
Runtime Values //
    ↓
Output / State Change

*/