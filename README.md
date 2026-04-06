# EzLang Compiler

A complete compiler/interpreter for **EzLang** - a simple, Bengali-friendly programming language designed for educational purposes. Built with Flex (lexer) and Bison (parser).

## ✨ Features

### Core Language Features

- **Data Types:** `num` (int), `dec` (double), `char`, `fixed` (const), `void`
- **Variables:** Declaration and assignment
- **Expressions:**
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Relational: `>`, `<`, `>=`, `<=`, `equals`, `differs`
  - Logical: `and`, `or`, `not`
  - Math functions: `power(x, y)`, `root(x)`
- **Control Flow:**
  - `check` / `or check` / `otherwise` (if/else if/else)
  - `during` (while loop)
  - `iterate(i = a to b)` (for loop)
- **I/O:**
  - `show(...)` - print output
  - `scan(...)` - read input
  - `send` - return value
- **Functions:** User-defined functions with parameters
- **Comments:** `# single-line` and `/* multi-line */`

### 🆕 C Code Generation

The compiler generates equivalent **C code** from EzLang source:

```
input.ez  →  input.c (compilable with gcc)
```

### 🆕 Three-Address Code (TAC)

Minimal TAC generation for arithmetic expressions (`+`, `-`, `*`, `/`):

Generates `test/tac_only.tac`:

```
t1 = b * 2
t2 = a + t1
t3 = b / 1
t4 = t2 - t3
c = t4
```

## 📁 Project Structure

```
EzLang/
├── ezlang.l          # Flex lexer specification
├── ezlang.y          # Bison parser + grammar rules
├── ast.h / ast.c     # Abstract Syntax Tree definitions
├── runtime.h / runtime.c   # Interpreter/executor
├── codegen.h / codegen.c   # C code generator (NEW!)
├── Makefile          # Build automation
├── test/             # Test files (.ez)
│   ├── for_loop.ez
│   ├── if_else.ez
│   ├── nested_loop.ez
│   ├── var_decl_assign.ez
│   ├── function_builtin.ez
│   ├── user_defined_function_simple.ez
│   └── ...
├── explain.md        # Detailed code documentation (Bengali)
└── README.md
```

## 🔧 Build

### Prerequisites

- `bison` (GNU Bison)
- `flex` (Fast Lexical Analyzer)
- `gcc` (GNU C Compiler)

### Using Makefile (Recommended)

Linux/macOS:

```bash
make build      # Build the compiler
make test       # Run all tests
make clean      # Clean generated files
```

Windows (cmd + MinGW):

```bat
mingw32-make build
mingw32-make test
mingw32-make clean
```

Note: `make test` stores per-test logs in `test/output/*.out`, so terminal output is intentionally minimal.

### Manual Build

```bash
bison -d ezlang.y
flex ezlang.l
gcc ezlang.tab.c lex.yy.c ast.c runtime.c codegen.c -o ezlang_parser.exe -lm
```

## 🚀 Usage

### Run EzLang Program

Linux/macOS:

```bash
./ezlang_parser test/for_loop.ez
```

Windows (cmd):

```bat
ezlang_parser.exe test\for_loop.ez
```

**Output:**

1. Token listing (lexical analysis)
2. Program execution output (interpreter)
3. Generated C file (`test/for_loop.c`)

### Save Output to File

Linux/macOS:

```bash
./ezlang_parser test/for_loop.ez output.txt
```

Windows (cmd):

```bat
ezlang_parser.exe test\for_loop.ez output.txt
```

### Compile and Run Generated C (Windows)

```bat
gcc test\for_loop.c -o test\for_loop.exe -lm
test\for_loop.exe
```

## 📝 Example

### EzLang Code (`test/for_loop.ez`)

```ezlang
start() begin
    iterate(i = 1 to 3) begin
        show(i)
    end
    send 0
end
```

### Generated C Code (`test/for_loop.c`)

```c
#include <stdio.h>
#include <math.h>

int main(void)
{
    for (int i = 1; i <= 3; i++)
    {
        printf("%g\n", (double)i);
    }
    return 0;
}
```

### Runtime Output

```
1
2
3
```

## 🧪 Test Files

| Test File                         | Description                                       |
| --------------------------------- | ------------------------------------------------- |
| `var_decl_assign.ez`              | Variable declaration & assignment                 |
| `for_loop.ez`                     | For loop (iterate)                                |
| `if_else.ez`                      | Conditional statements (check/or check/otherwise) |
| `nested_loop.ez`                  | Nested loops                                      |
| `function_builtin.ez`             | Built-in functions (power, root)                  |
| `user_defined_function_simple.ez` | User-defined functions                            |
| `tac_only.ez`                     | TAC generation demo (`EZLANG_EMIT_TAC=1`)         |
| `invalid_token.ez`                | Lexical error handling                            |
| `syntax_error.ez`                 | Syntax error handling                             |

## 📖 Documentation

- **`explain.md`** - Detailed line-by-line code explanation (Bengali)
- **`ezlang_syntax_cheatsheet_bn.md`** - EzLang syntax reference (Bengali)

## ⚙️ Compiler Pipeline

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Source    │────▶│    Lexer    │────▶│   Parser    │
│  (.ez file) │     │  (ezlang.l) │     │  (ezlang.y) │
└─────────────┘     └─────────────┘     └──────┬──────┘
                                               │
                                               ▼
                                         ┌─────────────┐
                                         │     AST     │
                                         │  (ast.c)    │
                                         └──────┬──────┘
                           ┌───────────────────┴───────────────────┐
                           ▼                                       ▼
                    ┌─────────────┐                         ┌─────────────┐
                    │   Runtime   │                         │   Codegen   │
                    │ (runtime.c) │                         │ (codegen.c) │
                    └──────┬──────┘                         └──────┬──────┘
                           ▼                                       ▼
                    ┌─────────────┐                         ┌─────────────┐
                    │   stdout    │                         │   .c file   │
                    │  (output)   │                         │ (generated) │
                    └─────────────┘                         └─────────────┘
```
