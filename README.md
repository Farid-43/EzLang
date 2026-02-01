# EzLang Lexical Analyzer

A lexical analyzer (scanner) for the **EzLang** programming language, built using Flex. This project is part of the Compiler Design Laboratory course (CSE 3212).

## 🎯 Overview

EzLang is a beginner-friendly programming language with English-like keywords. This lexical analyzer breaks down EzLang source code into tokens for further processing by a parser.

## 🚀 Features

### Supported EzLang Features:

- **Data Types**: `num`, `dec`, `char`, `fixed`, `void`
- **Control Structures**: `check`/`or check`/`otherwise` (if-else), `inspect`/`option`/`fallback` (switch-case)
- **Loops**: `during` (while), `iterate` (for)
- **I/O Operations**: `show` (print), `scan` (input), `send` (return)
- **Logical Operators**: `and`, `or`, `not`, `equals`, `differs`
- **Math Functions**: `power`, `root`
- **Comments**: Single-line (`#`) and multi-line (`/* */`)

### Technical Features:

- Automatic line number tracking (`%option yylineno`)
- Multi-line comment state machine
- Regular expression definitions for maintainability
- Flexible output (screen or file)
- Command-line argument support

## 📁 Project Structure

```
EzLang/
├── ezlang.l          # Flex lexer specification
├── test.ez           # Sample EzLang test file
├── lex.yy.c          # Generated C scanner (after compilation)
├── ezlang_lexer.exe  # Executable (after compilation)
└── README.md         # This file
```

## 🛠️ Build Instructions

### Prerequisites:

- **Flex** (lexical analyzer generator)
- **GCC** (C compiler)

### Compilation:

```bash
# Step 1: Generate C scanner from Flex file
flex ezlang.l

# Step 2: Compile the generated C code
gcc lex.yy.c -o ezlang_lexer

# Step 3: Run the lexer
ezlang_lexer test.ez

# Alternative: One-line build
flex ezlang.l && gcc lex.yy.c -o ezlang_lexer
```

## 📖 Usage

### Basic Usage:

```bash
# Analyze from keyboard input
./ezlang_lexer

# Analyze a file and display results
./ezlang_lexer test.ez

# Analyze a file and save results
./ezlang_lexer test.ez output.txt
```

### Command-line Arguments:

```bash
ezlang_lexer [input_file] [output_file]
```

- `input_file`: EzLang source file to analyze (optional, defaults to stdin)
- `output_file`: Where to save results (optional, defaults to stdout)

## 💡 Example

### Sample EzLang Code (`test.ez`):

```ezlang
# Simple EzLang Program
start() begin
    num x = 5
    check (x > 0) begin
        show("Positive number")
    end
    send 0
end
```

### Output:

```
================================================================
  EzLang Lexical Analyzer
  Compiler Design Laboratory - CSE 3212
  Author: Farid Ahmed Patwary (Roll: 2107043)
================================================================

Analyzing file: test.ez

Line       Token Type                Lexeme
---------------------------------------------------------------
Line   2: KEYWORD_START             --> start
Line   2: LEFT_PAREN                --> (
Line   2: RIGHT_PAREN               --> )
Line   2: KEYWORD_BEGIN             --> begin
Line   3: KEYWORD_NUM               --> num
Line   3: IDENTIFIER                --> x
Line   3: ASSIGN_OP                 --> =
Line   3: INTEGER_LITERAL           --> 5
Line   4: KEYWORD_CHECK             --> check
Line   4: LEFT_PAREN                --> (
Line   4: IDENTIFIER                --> x
Line   4: GREATER_THAN              --> >
Line   4: INTEGER_LITERAL           --> 0
Line   4: RIGHT_PAREN               --> )
Line   4: KEYWORD_BEGIN             --> begin
Line   5: KEYWORD_SHOW              --> show
Line   5: LEFT_PAREN                --> (
Line   5: STRING_LITERAL            --> "Positive number"
Line   5: RIGHT_PAREN               --> )
Line   6: KEYWORD_END               --> end
Line   7: KEYWORD_SEND              --> send
Line   7: INTEGER_LITERAL           --> 0
Line   8: KEYWORD_END               --> end

================================================================
Lexical Analysis Complete!
Total Tokens Found: 19
Total Lines: 8
================================================================
```

## 🏗️ EzLang Language Specification

### Keywords:

| Category              | EzLang Keyword                  | Equivalent             |
| --------------------- | ------------------------------- | ---------------------- |
| **Program Structure** | `start`, `begin`, `end`         | main(), {, }           |
| **Data Types**        | `num`, `dec`, `char`, `void`    | int, float, char, void |
| **Control Flow**      | `check`, `otherwise`, `inspect` | if, else, switch       |
| **Loops**             | `during`, `iterate`             | while, for             |
| **I/O**               | `show`, `scan`, `send`          | print, input, return   |
| **Logic**             | `and`, `or`, `not`, `equals`    | &&, \|\|, !, ==        |

### Operators:

- **Arithmetic**: `+`, `-`, `*`, `/`, `%`
- **Comparison**: `>`, `<`, `>=`, `<=`, `equals`, `differs`
- **Assignment**: `=`
- **Math Functions**: `power()`, `root()`

## 🧪 Testing

The project includes comprehensive test cases in `test.ez` covering:

- Variable declarations and assignments
- Conditional statements (if-else)
- Loops (while, for)
- Function definitions
- All data types
- Switch statements
- Logical operations
- Mathematical operations

## 👨‍💻 Author

**Farid Ahmed Patwary**  
Roll: 2107043  
Course: CSE 3212 - Compiler Design Laboratory

## 📚 Technical Details

- **Lexer Generator**: GNU Flex
- **Language**: C
- **Line Tracking**: Automatic (`yylineno`)
- **State Management**: Multi-line comment handling
- **Memory Management**: Safe string handling
- **Error Handling**: Unrecognized character detection

---

_This lexical analyzer is the first phase of building a complete EzLang compiler._
