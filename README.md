# EzLang Compiler (Core Milestone)

This project currently completes the core compiler milestones using Flex + Bison:

- Lexical analysis (tokenization)
- Syntax analysis (grammar parsing)
- Runtime execution for core language constructs

Unique/advanced features are intentionally postponed for a later phase.

## Current Status

Core features implemented:

- Data types: `num`, `dec`, `char`, `fixed`, `void`
- Declarations and assignments
- Expressions:
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Relational: `>`, `<`, `>=`, `<=`, `equals`, `differs`
  - Logical: `and`, `or`, `not`
  - Math functions: `power()`, `root()`
- Control flow:
  - `check` / `or check` / `otherwise`
  - `during`
  - `iterate(i = a to b)`
- I/O:
  - `show(...)`
  - `scan(...)`
  - `send`
- Comments:
  - single-line `# ...`
  - multi-line `/* ... */`

## Project Files

- `ezlang.l` - Flex lexer specification
- `ezlang.y` - Bison parser + AST + executor
- `test/test.ez` - main core test program
- `test/test_core.ez` - additional core behavior regression test

Generated files after build:

- `ezlang.tab.c`, `ezlang.tab.h`
- `lex.yy.c`
- `ezlang_parser.exe`

## Build and Run (Windows)

Use a shell where `bison`, `flex`, and `gcc` are available.

```bat
bison -d ezlang.y
flex ezlang.l
gcc ezlang.tab.c lex.yy.c -o ezlang_parser.exe -lm
```

Run with input file:

```bat
ezlang_parser.exe test\test.ez
```

Or write output to a file:

```bat
ezlang_parser.exe test\test.ez output.txt
```

## Verified Core Tests

1. `test/test.ez`

- Validates declarations, expressions, `check`, `during`, `show`, and `send`.

2. `test/test_core.ez`

- Validates newline-separated `check -> or check -> otherwise` chain.
- Validates `iterate` and `during` loops.

## Notes

- Parser currently builds with expected grammar conflicts (`%expect 5`).
- This is acceptable for the current grammar shape and does not block core behavior.
- Advanced/unique rubric items (for example, TAC generation or extra optimizations) are not included yet by design.
