%option noyywrap nodefault nounput noinput yylineno
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include "ast.h"
#include "parser.h"

extern YYSTYPE yylval;

#undef YY_DECL
#define YY_DECL static int yylex_internal(void)

static int indent_stack[1024] = {0};
static int indent_top = 0;

static int token_queue[1024];
static int queue_head = 0;
static int queue_tail = 0;

static int pending_indent = -1;
static bool at_line_start = true;

static char *dup_text(const char *src) {
    size_t len = strlen(src);
    char *copy = malloc(len + 1);
    if (!copy) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    memcpy(copy, src, len + 1);
    return copy;
}

static void enqueue_token(int token) {
    int next_tail = (queue_tail + 1) % 1024;
    if (next_tail == queue_head) {
        fprintf(stderr, "Token queue overflow\n");
        exit(EXIT_FAILURE);
    }
    token_queue[queue_tail] = token;
    queue_tail = next_tail;
}

static bool has_pending_tokens(void) {
    return queue_head != queue_tail;
}

static int dequeue_token(void) {
    if (!has_pending_tokens()) {
        return 0;
    }
    int token = token_queue[queue_head];
    queue_head = (queue_head + 1) % 1024;
    return token;
}

static void handle_indent(int indent_level) {
    int current = indent_stack[indent_top];
    if (indent_level > current) {
        indent_top++;
        indent_stack[indent_top] = indent_level;
        enqueue_token(T_INDENT);
    } else if (indent_level < current) {
        while (indent_top > 0 && indent_level < indent_stack[indent_top]) {
            enqueue_token(T_DEDENT);
            indent_top--;
        }
        if (indent_stack[indent_top] != indent_level) {
            fprintf(stderr, "Indentation error: inconsistent dedent to %d (have %d)\n",
                    indent_level, indent_stack[indent_top]);
            exit(EXIT_FAILURE);
        }
    }
}

static void prepare_indent_tokens(void) {
    if (at_line_start) {
        if (pending_indent >= 0) {
            handle_indent(pending_indent);
            pending_indent = -1;
        }
        at_line_start = false;
    }
}

static char decode_escape(char c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case '"': return '"';
        case '\\': return '\\';
        default: return c;
    }
}

%}

%%

"conta"        { prepare_indent_tokens(); return T_CONTA; }
"se"           { prepare_indent_tokens(); return T_SE; }
"senão"        { prepare_indent_tokens(); return T_SENAO; }
"enquanto"     { prepare_indent_tokens(); return T_ENQUANTO; }
"depositar"    { prepare_indent_tokens(); return T_DEPOSITAR; }
"sacar"        { prepare_indent_tokens(); return T_SACAR; }
"transferir"   { prepare_indent_tokens(); return T_TRANSFERIR; }
"aplicar_juros" { prepare_indent_tokens(); return T_APLICAR_JUROS; }
"mostrar"      { prepare_indent_tokens(); return T_MOSTRAR; }
"tempo"        { prepare_indent_tokens(); return T_TEMPO; }
"juros"        { prepare_indent_tokens(); return T_JUROS; }
"verdadeiro"   { prepare_indent_tokens(); return T_VERDADEIRO; }
"falso"        { prepare_indent_tokens(); return T_FALSO; }

[0-9]+(\.[0-9]+)? {
    prepare_indent_tokens();
    yylval.number = strtod(yytext, NULL);
    return T_NUMBER;
}

[A-Za-z_][A-Za-z0-9_]* {
    prepare_indent_tokens();
    yylval.string = dup_text(yytext);
    return T_IDENTIFIER;
}

\"([^\\\n]|\\.)*\" {
    prepare_indent_tokens();
    size_t len = yyleng;
    char *result = malloc(len); /* len includes quotes, result len <= len-2 + 1 */
    if (!result) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    size_t out_idx = 0;
    for (size_t i = 1; i < len - 1; ++i) {
        if (yytext[i] == '\\') {
            ++i;
            result[out_idx++] = decode_escape(yytext[i]);
        } else {
            result[out_idx++] = yytext[i];
        }
    }
    result[out_idx] = '\0';
    yylval.string = result;
    return T_STRING;
}

"=="           { prepare_indent_tokens(); return T_EQEQ; }
"!="           { prepare_indent_tokens(); return T_NEQ; }
"<="           { prepare_indent_tokens(); return T_LTE; }
">="           { prepare_indent_tokens(); return T_GTE; }

"="            { prepare_indent_tokens(); return '='; }
"+"            { prepare_indent_tokens(); return '+'; }
"-"            { prepare_indent_tokens(); return '-'; }
"*"            { prepare_indent_tokens(); return '*'; }
"/"            { prepare_indent_tokens(); return '/'; }
"%"            { prepare_indent_tokens(); return '%'; }
"("            { prepare_indent_tokens(); return '('; }
")"            { prepare_indent_tokens(); return ')'; }
","            { prepare_indent_tokens(); return ','; }
"!"            { prepare_indent_tokens(); return '!'; }
"<"            { prepare_indent_tokens(); return '<'; }
">"            { prepare_indent_tokens(); return '>'; }

\r?\n[ \t]* {
    size_t len = yyleng;
    size_t indent = 0;
    for (size_t i = len; i > 0; --i) {
        char c = yytext[i - 1];
        if (c == ' ') {
            indent++;
        } else if (c == '\t') {
            fprintf(stderr, "Tabs are not allowed in indentation (line %d)\n", yylineno);
            exit(EXIT_FAILURE);
        } else {
            break;
        }
    }
    pending_indent = (int)indent;
    at_line_start = true;
    return T_NEWLINE;
}

[ \t]+  ; /* ignore in-line whitespace */

"#"[^\n]*       ; /* ignore comments */

. {
    fprintf(stderr, "Unexpected character '%s' on line %d\n", yytext, yylineno);
    exit(EXIT_FAILURE);
}

%%

#undef yylex

int yylex(void) {
    if (has_pending_tokens()) {
        return dequeue_token();
    }

    int token = yylex_internal();

    if (has_pending_tokens()) {
        if (token != 0) {
            enqueue_token(token);
        }
        return dequeue_token();
    }

    if (token == 0) {
        if (pending_indent >= 0) {
            handle_indent(pending_indent);
            pending_indent = -1;
        }
        if (indent_stack[indent_top] != 0) {
            handle_indent(0);
        }
    }

    if (has_pending_tokens()) {
        return dequeue_token();
    }

    return token;
}
