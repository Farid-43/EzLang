#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "runtime.h"

/*
 * Code Generator for EzLang
 * Generates equivalent C code from AST
 */

/* Generate C code for the entire program */
void generateCCode(FILE *out, Node *main_block);
/*
- `out` → output file (যেখানে C code লেখা হবে)
- `main_block` → main() function এর AST (block of statements)
*/

/* Register a user-defined function for code generation */
void registerFuncForCodegen(const char *name, int return_type, Node *params, Node *body);

#endif
