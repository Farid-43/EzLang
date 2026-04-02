%option yylineno

%{
#include <stdio.h>
#include <string.h>
#include "ezlang.tab.h"

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

"use library"       { print_token("INCLUDE", yytext); return INCLUDE; }
"start"             { print_token("MAIN", yytext); return MAIN; }
"begin"             { print_token("LBRACE", yytext); return LBRACE; }
"end"               { print_token("RBRACE", yytext); return RBRACE; }

"num"               { print_token("INT", yytext); return INT; }
"dec"               { print_token("DOUBLE", yytext); return DOUBLE; }
"char"              { print_token("CHAR", yytext); return CHAR; }
"fixed"             { print_token("CONST", yytext); return CONST; }
"void"              { print_token("VOID", yytext); return VOID; }

"check"             { print_token("IF", yytext); return IF; }
"or check"          { print_token("ELSE_IF", yytext); return ELSE_IF; }
"otherwise"         { print_token("ELSE", yytext); return ELSE; }
"inspect"           { print_token("SWITCH", yytext); return SWITCH; }
"option"            { print_token("CASE", yytext); return CASE; }
"fallback"          { print_token("DEFAULT", yytext); return DEFAULT; }
"stop"              { print_token("BREAK", yytext); return BREAK; }
"skip"              { print_token("CONTINUE", yytext); return CONTINUE; }

"iterate"           { print_token("FOR", yytext); return FOR; }
"during"            { print_token("WHILE", yytext); return WHILE; }
"to"                { print_token("TO", yytext); return TO; }

"send"              { print_token("RETURN", yytext); return RETURN; }
"show"              { print_token("PRINTF", yytext); return PRINTF; }
"scan"              { print_token("SCANF", yytext); return SCANF; }

"and"               { print_token("AND", yytext); return AND; }
"or"                { print_token("OR", yytext); return OR; }
"not"               { print_token("NOT", yytext); return NOT; }
"equals"            { print_token("EQ", yytext); return EQ; }
"differs"           { print_token("NEQ", yytext); return NEQ; }

"power"             { print_token("POW", yytext); return POW; }
"root"              { print_token("SQRT", yytext); return SQRT; }


"="                 { print_token("ASSIGN", yytext); return ASSIGN; }
"+"                 { print_token("PLUS", yytext); return PLUS; }
"-"                 { print_token("MINUS", yytext); return MINUS; }
"*"                 { print_token("MULT", yytext); return MULT; }
"/"                 { print_token("DIV", yytext); return DIV; }
"%"                 { print_token("MOD", yytext); return MOD; }
">="                { print_token("GE", yytext); return GE; }
"<="                { print_token("LE", yytext); return LE; }
">"                 { print_token("GT", yytext); return GT; }
"<"                 { print_token("LT", yytext); return LT; }


"("                 { print_token("LPAREN", yytext); return LPAREN; }
")"                 { print_token("RPAREN", yytext); return RPAREN; }
","                 { print_token("COMMA", yytext); return COMMA; }
":"                 { print_token("COLON", yytext); return COLON; }

{FLOAT}             { print_token("DOUBLE_LIT", yytext); yylval.dval = atof(yytext); return DOUBLE_LIT; }
{INTEGER}           { print_token("INT_LIT", yytext); yylval.dval = atof(yytext); return INT_LIT; }
'[^']'              { print_token("CHAR_LIT", yytext); yylval.cval = yytext[1]; return CHAR_LIT; }
\"[^\"]*\"        { char *s; print_token("STRING_LIT", yytext); s = strdup(yytext + 1); s[strlen(s) - 1] = '\0'; yylval.sval = s; return STRING_LIT; }

{IDENTIFIER}        { print_token("IDENTIFIER", yytext); yylval.sval = strdup(yytext); return IDENTIFIER; }

{WHITESPACE}        { }
{NEWLINE}           { return NEWLINE; }

.                   {
                        fprintf(yyout, "Line %d: ERROR - Unrecognized character: '%s'\n", yylineno, yytext);
                        return INVALID;
                    }

%%

void print_token(const char* token_type, const char* lexeme) {
    token_count++;
    fprintf(yyout, "Line %3d: %-25s --> %s\n", yylineno, token_type, lexeme);
}

int yywrap() {
    return 1;
}
