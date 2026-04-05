%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "runtime.h"
#include "codegen.h"

int  yylex(void);
void yyerror(const char *s);

extern int   yylineno;
extern FILE *yyin;
extern FILE *yyout;

/* For code generation */
static Node *main_block_for_codegen = NULL;
static char *codegen_output_file = NULL;
%}

%union {
    double      dval;
    char        cval;
    char       *sval;
    struct Node *node;
    int         ival;
}

%token NEWLINE INVALID
%token INCLUDE MAIN LBRACE RBRACE
%token INT DOUBLE CHAR CONST VOID
%token IF ELSE_IF ELSE
%token SWITCH CASE DEFAULT BREAK CONTINUE
%token FOR WHILE TO
%token RETURN PRINTF SCANF
%token AND OR NOT EQ NEQ
%token POW SQRT
%token ASSIGN PLUS MINUS MULT DIV MOD
%token GE LE GT LT
%token LPAREN RPAREN COMMA COLON

%token <dval> INT_LIT DOUBLE_LIT
%token <cval> CHAR_LIT
%token <sval> STRING_LIT IDENTIFIER

%type <node> block statements_opt statements statement
%type <node> declaration assignment show_stmt send_stmt
%type <node> check_stmt check_tail during_stmt iterate_stmt
%type <node> expression literal arg_list arg_list_opt
%type <node> param_defs_opt param_defs param_def
%type <ival>  type

%start program
%expect 5

/*
Associativity given.
*/

%left  OR
%left  AND
%right NOT
%left  EQ NEQ GT LT GE LE
%left  PLUS MINUS
%left  MULT DIV MOD
%right UMINUS

%%

program
        : opt_newlines opt_imports opt_func_defs opt_newlines MAIN LPAREN RPAREN LBRACE block RBRACE opt_newlines
      {
                /* Execute main block: runtime statements write directly to yyout. */
                    execBlock($9);
          
          /* Save main block for code generation -> .c file output*/
          main_block_for_codegen = $9;
      }
    ;

opt_imports
    : /* empty */
    | opt_imports INCLUDE opt_newlines
    ;

opt_func_defs
    : /* empty */
    | opt_func_defs opt_newlines function_def
    ;

function_def
    : type IDENTIFIER LPAREN param_defs_opt RPAREN LBRACE block RBRACE
      {
          if (findFunc($2)) {
              fprintf(stderr, "Error: duplicate function '%s'\n", $2);
          } else {
              addFunc($2, $1, $4, $7);      /* for runtimr register */
              /*
                add(num a, num b) begin
                    send a + b
                end

                Semantic Action:
                - `$1` = return type (VAL_NUM)
                - `$2` = function name ("add")
                - `$4` = parameter list (linked list of N_DECL nodes)
                - `$7` = function body (linked list of statements)
              */
              registerFuncForCodegen($2, $1, $4, $7);  /* For C code generation */
          }
      }
    ;

param_defs_opt
    : /* empty */     { $$ = NULL; }
    | param_defs      { $$ = $1;   }
    ;

param_defs
    : param_def                       { $$ = $1; }
    | param_defs COMMA param_def      { $$ = append($1, $3); }
    ;
 
param_def
    : type IDENTIFIER
      { Node *n = makeNode(N_DECL); n->vartype = $1; n->sval = $2; n->left = NULL; $$ = n; }
    ;

block
    : opt_newlines statements_opt   { $$ = $2; }
    ;

statements_opt
    : /* empty */         { $$ = NULL; }
    | statements          { $$ = $1;   }
    ;

statements
    : statement opt_newlines              { $$ = $1; }
    | statements statement opt_newlines   { $$ = append($1, $2); }
    ;
/*
num x = 1
num y = 2
show(x + y)
    ↓
N_DECL → N_DECL → N_SHOW → NULL

- `block` = newlines দিয়ে শুরু, তারপর statements
- `statements` recursively `append()` করে linked list বানায়
*/

opt_newlines
    : /* empty */
    | opt_newlines NEWLINE
    ;

statement
    : declaration   { $$ = $1; }
    | assignment    { $$ = $1; }
    | show_stmt     { $$ = $1; }
    | send_stmt     { $$ = $1; }
    | check_stmt    { $$ = $1; }
    | during_stmt   { $$ = $1; }
    | iterate_stmt  { $$ = $1; }
    ;

type
    : INT    { $$ = VAL_NUM;  }
    | DOUBLE { $$ = VAL_DEC;  }
    | CHAR   { $$ = VAL_CHAR; }
    | VOID   { $$ = VAL_NUM;  }
    ;

declaration
    : type IDENTIFIER
      { Node *n = makeNode(N_DECL); n->vartype=$1; n->sval=$2; n->left=NULL;  $$ = n; }
    | type IDENTIFIER ASSIGN expression
      { Node *n = makeNode(N_DECL); n->vartype=$1; n->sval=$2; n->left=$4;    $$ = n; }
    | CONST type IDENTIFIER ASSIGN expression
      { Node *n = makeNode(N_DECL); n->vartype=$2; n->sval=$3; n->left=$5; n->is_fixed=1; $$ = n; }
    ;
/*
3 Forms:

| Form          | Example               | Node Fields                          |
| ------------- | --------------------- | ------------------------------------ |
| Uninitialized | `num x`               | vartype, sval, left=NULL             |
| Initialized   | `num x = 5`           | vartype, sval, left=expression       |
| Constant      | `fixed num PI = 3.14` | vartype, sval, left=expr, is_fixed=1 |
*/
assignment
    : IDENTIFIER ASSIGN expression
      { Node *n = makeNode(N_ASSIGN); n->sval=$1; n->left=$3; $$ = n; }
    ;
/*
Example: `x = x + 1`

N_ASSIGN {
    sval: "x",
    left: N_BINOP("+") {
        left: N_ID("x"),
        right: N_NUM(1)
    }
}
*/

show_stmt
    : PRINTF LPAREN arg_list_opt RPAREN
      { Node *n = makeNode(N_SHOW); n->left=$3; $$ = n; }
    ;

send_stmt
    : RETURN
      { Node *n = makeNode(N_SEND); n->left=NULL; $$ = n; }
    | RETURN expression
      { Node *n = makeNode(N_SEND); n->left=$2;  $$ = n; }
    ;
/*
- `send` → program exit / function return (no value)
- `send 0` → return with value
*/
check_stmt
        : IF LPAREN expression RPAREN LBRACE block RBRACE opt_newlines check_tail
            { Node *n = makeNode(N_CHECK); n->left=$3; n->right=$6; n->extra=$9; $$ = n; }
    ;

check_tail
    : /* empty */
      { $$ = NULL; }
        | ELSE_IF LPAREN expression RPAREN LBRACE block RBRACE opt_newlines check_tail
            { Node *n = makeNode(N_CHECK); n->left=$3; n->right=$6; n->extra=$9; $$ = n; }
        | ELSE LBRACE block RBRACE
            { Node *n = makeNode(N_CHECK); n->left=NULL; n->right=$3; n->extra=NULL; $$ = n; }
    ;
/*
If-Else Chain Structure:

check (x > 0) begin
    show("positive")
end
or check (x < 0) begin
    show("negative")
end
otherwise begin
    show("zero")
end

AST Structure:

N_CHECK (if x > 0)
├── left: condition (x > 0)
├── right: body ("positive")
└── extra: N_CHECK (else if x < 0)
           ├── left: condition (x < 0)
           ├── right: body ("negative")
           └── extra: N_CHECK (else)
                      ├── left: NULL (no condition)
                      ├── right: body ("zero")
                      └── extra: NULL

*/
during_stmt
    : WHILE LPAREN expression RPAREN LBRACE block RBRACE
      { Node *n = makeNode(N_DURING); n->left=$3; n->right=$6; $$ = n; }
    ;
/*
Example: `during (x < 10) begin ... end`

N_DURING {
    left: condition (x < 10),
    right: body (statements)
}
*/

iterate_stmt
    : FOR LPAREN IDENTIFIER ASSIGN expression TO expression RPAREN LBRACE block RBRACE
      { Node *n = makeNode(N_ITERATE); n->sval=$3; n->left=$5; n->right=$7; n->extra=$10; $$ = n; }
    ;
/*
Example: `iterate(i = 1 to 5) begin ... end`

N_ITERATE {
    sval: "i",       // loop variable
    left: 1,         // start value
    right: 5,        // end value
    extra: body      // loop body
}
*/
arg_list_opt
    : /* empty */             { $$ = NULL; }
    | arg_list                { $$ = $1;   }
    ;

arg_list    /* Function call বা show() এর arguments এর list build করে। */
    : expression                    { $$ = $1; }
    | arg_list COMMA expression     { $$ = append($1, $3); }
    ;

expression
    : literal                                                        { $$ = $1; }
    | IDENTIFIER LPAREN arg_list_opt RPAREN                          { Node *n=makeNode(N_CALL); n->sval=$1; n->left=$3; $$ = n; }
    | IDENTIFIER                                                     { $$ = makeId($1); }
    | SCANF LPAREN RPAREN
      { Node *n=makeNode(N_SCAN); n->sval=NULL; $$ = n; }
    | SCANF LPAREN IDENTIFIER RPAREN
      { Node *n=makeNode(N_SCAN); n->sval=$3;   $$ = n; }
    | LPAREN expression RPAREN                              { $$ = $2; }
    | POW LPAREN expression COMMA expression RPAREN  { $$ = makeBinop("power", $3, $5); }
    | SQRT  LPAREN expression RPAREN                   { $$ = makeUnop("root",  $3); }
    | NOT expression                                         { $$ = makeUnop("not",   $2); }
    | MINUS expression %prec UMINUS                               { $$ = makeUnop("neg",   $2); }
    | expression PLUS  expression    { $$ = makeBinop("+",   $1, $3); }
    | expression MINUS expression    { $$ = makeBinop("-",   $1, $3); }
    | expression MULT  expression    { $$ = makeBinop("*",   $1, $3); }
    | expression DIV   expression    { $$ = makeBinop("/",   $1, $3); }
    | expression MOD   expression    { $$ = makeBinop("%",   $1, $3); }
    | expression GT  expression   { $$ = makeBinop(">",  $1, $3); }
    | expression LT     expression   { $$ = makeBinop("<",  $1, $3); }
    | expression GE expression   { $$ = makeBinop(">=", $1, $3); }
    | expression LE    expression   { $$ = makeBinop("<=", $1, $3); }
    | expression EQ  expression  { $$ = makeBinop("==", $1, $3); }
    | expression NEQ expression  { $$ = makeBinop("!=", $1, $3); }
    | expression AND expression   { $$ = makeBinop("and", $1, $3); }
    | expression OR  expression   { $$ = makeBinop("or",  $1, $3); }
    ;

literal /* Lexer থেকে আসা literal values কে AST nodes এ convert করে। */
    : INT_LIT       { $$ = makeNum($1); }
    | DOUBLE_LIT    { $$ = makeDec($1); }
    | CHAR_LIT      { $$ = makeChar($1); }
    | STRING_LIT    { $$ = makeStr($1);  }
    ;

%%

/* Runtime implementation moved to runtime.c */

/* ===== yyerror + main ===== */

void yyerror(const char *s) {
    fprintf(stderr, "Line %d: Syntax error: %s\n", yylineno, s);
}

int main(int argc, char **argv) {
    FILE *input_file = NULL;
    char *input_filename = NULL;

    yyout = stdout;
    if (argc > 2) {
        yyout = fopen(argv[2], "w");
        if (!yyout) { fprintf(stderr, "Cannot open %s\n", argv[2]); yyout = stdout; }
    }

    if (argc > 1) {
        input_filename = argv[1];
        input_file = fopen(argv[1], "r");
        if (!input_file) { fprintf(stderr, "Cannot open %s\n", argv[1]); return 1; }
        yyin = input_file;
    }

    if (yyparse() != 0) {
        fprintf(yyout, "\nParsing failed.\n");
    } else {
        /* Generate C code if parsing succeeded */
        if (main_block_for_codegen && input_filename) {
            char c_filename[512];
            FILE *c_file;
            char *dot;
            
            /* Create .c filename from input filename */
            strncpy(c_filename, input_filename, sizeof(c_filename) - 3);
            c_filename[sizeof(c_filename) - 3] = '\0';
            
            /* Replace .ez with .c */
            dot = strrchr(c_filename, '.');
            if (dot) {
                strcpy(dot, ".c");
            } else {
                strcat(c_filename, ".c");
            }
            
            c_file = fopen(c_filename, "w");
            if (c_file) {
                generateCCode(c_file, main_block_for_codegen);
                fclose(c_file);
            } else {
                fprintf(stderr, "Warning: Could not create %s\n", c_filename);
            }
        }
    }

    if (yyout != stdout) fclose(yyout);
    if (input_file) fclose(input_file);

    return 0;
}

/* Flow
┌───────────────────────────────────────────────────────────────┐
│                     main() Function Flow                       │
├───────────────────────────────────────────────────────────────┤
│                                                               │
│   1. File Setup                                               │
│      ├── input_filename = argv[1]  ← "test/for_loop.ez"       │
│      ├── yyin = fopen(input_filename)                         │
│      └── yyout = stdout or file                               │
│                                                               │
│   2. yyparse()                                                │
│      ├── Lexer (yylex) → tokens                               │
│      ├── Parser → AST                                         │
│      ├── execBlock($9) → runtime output                       │
│      └── main_block_for_codegen = $9  ← AST save              │
│                                                               │
│   3. Code Generation (if parsing succeeded)                   │
│      │                                                        │
│      ├── strncpy(c_filename, input_filename)                  │
│      │   └── "test/for_loop.ez"                               │
│      │                                                        │
│      ├── dot = strrchr(c_filename, '.')                       │
│      │   └── Points to ".ez"                                  │
│      │                                                        │
│      ├── strcpy(dot, ".c")                                    │
│      │   └── "test/for_loop.c"                                │
│      │                                                        │
│      ├── c_file = fopen(c_filename, "w")                      │
│      │   └── Create output file                               │
│      │                                                        │
│      └── generateCCode(c_file, main_block_for_codegen)        │
│          └── AST → C code লিখো                                │
│                                                               │
│   4. Cleanup                                                  │
│      └── fclose() all files                                   │
│                                                               │
└───────────────────────────────────────────────────────────────┘

Parser Flow (Updated with Code Generation):

Source Code (.ez)
    ↓ yylex() (Lexer)
Tokens
    ↓ yyparse() (Parser)
Grammar Rules match
    ↓ Semantic Actions
AST Nodes created
    ↓ program rule action
    ├── execBlock($9)              → Runtime Output (stdout)
    └── main_block_for_codegen=$9  → Save for codegen
    ↓ main() after yyparse()
generateCCode()
    ↓
Generated .c file
*/