# EzLang Simple Tests

This folder contains centralized EzLang test/input files.

## Files

1. `for_loop.ez`

- Simple `iterate` loop test.

2. `nested_loop.ez`

- Nested loop test (`iterate` + inner `during`).

3. `if_else.ez`

- Simple `check / or check / otherwise` test.

4. `var_decl_assign.ez`

- Variable declaration and assignment test.

5. `function_builtin.ez`

- Function-style built-in call test (`power`, `root`).

6. `user_defined_function_simple.ez`

- Simple user-defined function declaration/call test (no recursion).

7. `test.ez`

- Main core behavior test.

8. `test_core.ez`

- Additional core regression test.

9. `test_ops.ez`

- Arithmetic, comparison, and assignment checks.

10. `test_double_equal.ez`

- Negative syntax case (`==`) for current language style.

11. `invalid_token.ez`

- Negative lexical case (invalid character).

12. `syntax_error.ez`

- Negative parser case (incomplete statement).

## Run Example

```bat
bison -d ezlang.y
flex ezlang.l
gcc ezlang.tab.c lex.yy.c -o ezlang_parser.exe -lm

ezlang_parser.exe test\for_loop.ez
ezlang_parser.exe test\nested_loop.ez
ezlang_parser.exe test\if_else.ez
ezlang_parser.exe test\var_decl_assign.ez
ezlang_parser.exe test\function_builtin.ez
ezlang_parser.exe test\user_defined_function_simple.ez
ezlang_parser.exe test\test.ez
ezlang_parser.exe test\test_core.ez
ezlang_parser.exe test\test_ops.ez

:: Expected parse failures / negative tests
ezlang_parser.exe test\test_double_equal.ez
ezlang_parser.exe test\invalid_token.ez
ezlang_parser.exe test\syntax_error.ez
```

## Note

Current grammar supports:

1. Built-in function-style calls (`power`, `root`, `scan`)
2. Simple user-defined function declaration/call

No-recursion mode:

- User-defined functions cannot call other user-defined functions (including themselves).
