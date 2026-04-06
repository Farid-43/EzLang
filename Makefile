BISON ?= bison
FLEX ?= flex
CC = gcc
LDFLAGS ?=
LDLIBS ?= -lm

PARSER := ezlang_parser.exe
YACC_SRC := ezlang.y
LEX_SRC := ezlang.l
AST_SRC := ast.c
RUNTIME_SRC := runtime.c
CODEGEN_SRC := codegen.c
YACC_C := ezlang.tab.c
YACC_H := ezlang.tab.h
LEX_C := lex.yy.c
OUT_DIR := test/output
POSITIVE_TESTS := for_loop nested_loop if_else var_decl_assign function_builtin user_defined_function_simple nested_function
NEGATIVE_TESTS := invalid_token syntax_error
TAC_TEST := tac_only

.DEFAULT_GOAL := all

.PHONY: all build run test clean help

all: build

build: $(PARSER)

$(YACC_C) $(YACC_H): $(YACC_SRC)
	$(BISON) -d $(YACC_SRC)

$(LEX_C): $(LEX_SRC) $(YACC_H)
	$(FLEX) $(LEX_SRC)

$(PARSER): $(YACC_C) $(LEX_C) $(AST_SRC) $(RUNTIME_SRC) $(CODEGEN_SRC)
	$(CC) $(YACC_C) $(LEX_C) $(AST_SRC) $(RUNTIME_SRC) $(CODEGEN_SRC) -o $(PARSER) $(LDFLAGS) $(LDLIBS)

run: build
	./$(PARSER) test/nested_user_call.ez

test: build
	@mkdir -p $(OUT_DIR)
	@set -e; \
	for t in $(POSITIVE_TESTS); do \
		./$(PARSER) test/$$t.ez > $(OUT_DIR)/$$t.out 2>&1; \
	done; \
	for t in $(NEGATIVE_TESTS); do \
		./$(PARSER) test/$$t.ez > $(OUT_DIR)/$$t.out 2>&1 || true; \
	done; \
	EZLANG_EMIT_TAC=1 ./$(PARSER) test/$(TAC_TEST).ez > $(OUT_DIR)/$(TAC_TEST).out 2>&1

clean:
	$(RM) $(PARSER) $(YACC_C) $(YACC_H) $(LEX_C)
	rm -rf $(OUT_DIR)
	rm -f test/*.c
	rm -f test/*.tac
	rm -f test/*.exe

help:
	@echo Available targets:
	@echo   make build        - Generate parser executable
	@echo "  make run          - Run nested function call demo (test/nested_user_call.ez)"
	@echo   make test         - Run all tests and store logs in test/output
	@echo   make clean        - Remove generated build artifacts, test/output, and generated .c/.exe test files
