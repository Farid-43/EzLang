%option yylineno

%{
#include <stdio.h>
#include <string.h>

int token_count = 0;

void print_token(const char* token_type, const char* lexeme);
%}

DIGIT       [0-9]
LETTER      [a-zA-Z]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT}|_)*
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
WHITESPACE  [ \t]+
NEWLINE     \n

%x COMMENT

%%

"#".*               { }
"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { }
<COMMENT>.          { }

"use library"       { print_token("KEYWORD_IMPORT", yytext); return 1; }
"start"             { print_token("KEYWORD_START", yytext); return 2; }
"begin"             { print_token("KEYWORD_BEGIN", yytext); return 3; }
"end"               { print_token("KEYWORD_END", yytext); return 4; }

"num"               { print_token("KEYWORD_NUM", yytext); return 5; }
"dec"               { print_token("KEYWORD_DEC", yytext); return 6; }
"char"              { print_token("KEYWORD_CHAR", yytext); return 7; }
"fixed"             { print_token("KEYWORD_FIXED", yytext); return 8; }
"void"              { print_token("KEYWORD_VOID", yytext); return 9; }

"check"             { print_token("KEYWORD_CHECK", yytext); return 10; }
"or check"          { print_token("KEYWORD_OR_CHECK", yytext); return 11; }
"otherwise"         { print_token("KEYWORD_OTHERWISE", yytext); return 12; }
"inspect"           { print_token("KEYWORD_INSPECT", yytext); return 13; }
"option"            { print_token("KEYWORD_OPTION", yytext); return 14; }
"fallback"          { print_token("KEYWORD_FALLBACK", yytext); return 15; }
"stop"              { print_token("KEYWORD_STOP", yytext); return 16; }
"skip"              { print_token("KEYWORD_SKIP", yytext); return 17; }

"iterate"           { print_token("KEYWORD_ITERATE", yytext); return 18; }
"during"            { print_token("KEYWORD_DURING", yytext); return 19; }
"to"                { print_token("KEYWORD_TO", yytext); return 20; }

"send"              { print_token("KEYWORD_SEND", yytext); return 21; }
"show"              { print_token("KEYWORD_SHOW", yytext); return 22; }
"scan"              { print_token("KEYWORD_SCAN", yytext); return 23; }

"and"               { print_token("LOGICAL_AND", yytext); return 24; }
"or"                { print_token("LOGICAL_OR", yytext); return 25; }
"not"               { print_token("LOGICAL_NOT", yytext); return 26; }
"equals"            { print_token("RELATIONAL_EQUALS", yytext); return 27; }
"differs"           { print_token("RELATIONAL_DIFFERS", yytext); return 28; }

"power"             { print_token("MATH_POWER", yytext); return 29; }
"root"              { print_token("MATH_ROOT", yytext); return 30; }


"="                 { print_token("ASSIGN_OP", yytext); return 31; }
"+"                 { print_token("PLUS_OP", yytext); return 32; }
"-"                 { print_token("MINUS_OP", yytext); return 33; }
"*"                 { print_token("MULT_OP", yytext); return 34; }
"/"                 { print_token("DIV_OP", yytext); return 35; }
"%"                 { print_token("MOD_OP", yytext); return 36; }
">"                 { print_token("GREATER_THAN", yytext); return 37; }
"<"                 { print_token("LESS_THAN", yytext); return 38; }
">="                { print_token("GREATER_EQUAL", yytext); return 39; }
"<="                { print_token("LESS_EQUAL", yytext); return 40; }

"("                 { print_token("LEFT_PAREN", yytext); return 41; }
")"                 { print_token("RIGHT_PAREN", yytext); return 42; }
","                 { print_token("COMMA", yytext); return 43; }
":"                 { print_token("COLON", yytext); return 44; }

{FLOAT}             { print_token("FLOAT_LITERAL", yytext); return 46; }
{INTEGER}           { print_token("INTEGER_LITERAL", yytext); return 45; }
'[^']'              { print_token("CHAR_LITERAL", yytext); return 47; }
\"[^\"]*\"          { print_token("STRING_LITERAL", yytext); return 48; }

{IDENTIFIER}        { print_token("IDENTIFIER", yytext); return 49; }

{WHITESPACE}        { }
{NEWLINE}           { }

.                   { fprintf(yyout, "Line %d: ERROR - Unrecognized character: '%s'\n", yylineno, yytext); }

%%

void print_token(const char* token_type, const char* lexeme) {
    token_count++;
    fprintf(yyout, "Line %3d: %-25s --> %s\n", yylineno, token_type, lexeme);
}

int main(int argc, char** argv) {
    FILE* input_file;

    // Set yyout to stdout by default, or to a file if specified
    yyout = stdout;
    if (argc > 2) {
        yyout = fopen(argv[2], "w");
        if (!yyout) {
            fprintf(stderr, "Error: Cannot open output file %s\n", argv[2]);
            yyout = stdout;
        }
    }

    fprintf(yyout, "\n================================================================\n");
    fprintf(yyout, "  EzLang Lexical Analyzer\n");
    fprintf(yyout, "  Compiler Design Laboratory - CSE 3212\n");
    fprintf(yyout, "  Author: Farid Ahmed Patwary (Roll: 2107043)\n");
    fprintf(yyout, "================================================================\n\n");

    if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (!input_file) {
            fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
            return 1;
        }
        yyin = input_file;
        fprintf(yyout, "Analyzing file: %s\n\n", argv[1]);
    }

    fprintf(yyout, "%-10s %-25s %s\n", "Line", "Token Type", "Lexeme");
    fprintf(yyout, "---------------------------------------------------------------\n");

    while (yylex() != 0) { }

    fprintf(yyout, "\n================================================================\n");
    fprintf(yyout, "Lexical Analysis Complete!\n");
    fprintf(yyout, "Total Tokens Found: %d\n", token_count);
    fprintf(yyout, "Total Lines: %d\n", yylineno);
    fprintf(yyout, "================================================================\n");

    if (yyout != stdout) {
        fclose(yyout);
    }
    if (yyin != stdin) {
        fclose(input_file);
    }

    return 0;
}

int yywrap() {
    return 1;
}
