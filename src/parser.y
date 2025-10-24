%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex(void);
extern int yylineno;
void yyerror(const char *msg);

ASTProgram *root_program = NULL;
%}

%union {
    double number;
    char *string;
    ASTExpr *expr;
    ASTStmt *stmt;
    ASTStmtList *stmt_list;
    ASTProgram *program;
    ASTPrintArg *print_arg;
    ASTPrintArgList *print_arg_list;
    ASTBinaryOp binary_op;
}

%token <string> T_IDENTIFIER
%token <number> T_NUMBER
%token <string> T_STRING
%token T_CONTA T_SE T_SENAO T_ENQUANTO
%token T_DEPOSITAR T_SACAR T_TRANSFERIR T_APLICAR_JUROS T_MOSTRAR
%token T_TEMPO T_JUROS
%token T_VERDADEIRO T_FALSO
%token T_NEWLINE T_INDENT T_DEDENT
%token T_EQEQ T_NEQ T_LTE T_GTE

%left '+' '-'
%left '*' '/' '%'
%right UMINUS '!'

%type <program> program
%type <stmt_list> statement_seq block statement_seq_opt
%type <stmt> statement var_decl assignment if_stmt while_stmt command
%type <expr> expression term factor primary condition sensor
%type <print_arg_list> print_args
%type <print_arg> print_arg
%type <binary_op> comparison_op
%type <stmt_list> opt_else

%start program

%%

program
    : optional_newlines statement_seq_opt
      {
          root_program = ast_program_new($2);
          $$ = root_program;
      }
    ;

statement_seq_opt
    : /* empty */
      {
          $$ = ast_stmt_list_new();
      }
    | statement_seq
      { $$ = $1; }
    ;

statement_seq
    : statement newline_group
      {
          ASTStmtList *list = ast_stmt_list_new();
          ast_stmt_list_append(list, $1);
          $$ = list;
      }
    | statement
      {
          ASTStmtList *list = ast_stmt_list_new();
          ast_stmt_list_append(list, $1);
          $$ = list;
      }
    | statement_seq statement newline_group
      {
          ast_stmt_list_append($1, $2);
          $$ = $1;
      }
    | statement_seq statement
      {
          ast_stmt_list_append($1, $2);
          $$ = $1;
      }
    ;

newline_group
    : T_NEWLINE
    | newline_group T_NEWLINE
    ;

optional_newlines
    : /* empty */
    | optional_newlines T_NEWLINE
    ;

statement
    : var_decl
    | assignment
    | if_stmt
    | while_stmt
    | command
    ;

var_decl
    : T_CONTA T_IDENTIFIER '=' expression
      {
          $$ = ast_var_decl_new($2, $4);
      }
    ;

assignment
    : T_IDENTIFIER '=' expression
      {
          $$ = ast_assignment_new($1, $3);
      }
    ;

if_stmt
    : T_SE '(' condition ')' T_NEWLINE block opt_else
      {
          $$ = ast_if_new($3, $6, $7);
      }
    ;

opt_else
    : /* empty */
      { $$ = NULL; }
    | T_SENAO T_NEWLINE block
      { $$ = $3; }
    ;

while_stmt
    : T_ENQUANTO '(' condition ')' T_NEWLINE block
      {
          $$ = ast_while_new($3, $6);
      }
    ;

block
    : T_INDENT optional_newlines statement_seq_opt T_DEDENT
      { $$ = $3; }
    ;

command
    : T_DEPOSITAR '(' T_IDENTIFIER ',' expression ')'
      { $$ = ast_command_deposit_new($3, $5); }
    | T_SACAR '(' T_IDENTIFIER ',' expression ')'
      { $$ = ast_command_withdraw_new($3, $5); }
    | T_TRANSFERIR '(' T_IDENTIFIER ',' T_IDENTIFIER ',' expression ')'
      { $$ = ast_command_transfer_new($3, $5, $7); }
    | T_APLICAR_JUROS '(' T_IDENTIFIER ',' expression ')'
      { $$ = ast_command_interest_new($3, $5); }
    | T_MOSTRAR '(' print_args ')'
      { $$ = ast_command_print_new($3); }
    ;

print_args
    : print_arg
      {
          ASTPrintArgList *list = ast_print_arg_list_new();
          ast_print_arg_list_append(list, $1);
          $$ = list;
      }
    | print_args ',' print_arg
      {
          ast_print_arg_list_append($1, $3);
          $$ = $1;
      }
    ;

print_arg
    : expression
      { $$ = ast_print_arg_expr_new($1); }
    | T_STRING
      { $$ = ast_print_arg_string_new($1); }
    ;

condition
    : expression comparison_op expression
      {
          $$ = ast_binary_new($2, $1, $3);
      }
    ;

comparison_op
    : T_EQEQ { $$ = BIN_EQ; }
    | T_NEQ  { $$ = BIN_NEQ; }
    | '<'    { $$ = BIN_LT; }
    | '>'    { $$ = BIN_GT; }
    | T_LTE  { $$ = BIN_LE; }
    | T_GTE  { $$ = BIN_GE; }
    ;

expression
    : expression '+' term
      { $$ = ast_binary_new(BIN_ADD, $1, $3); }
    | expression '-' term
      { $$ = ast_binary_new(BIN_SUB, $1, $3); }
    | term
      { $$ = $1; }
    ;

term
    : term '*' factor
      { $$ = ast_binary_new(BIN_MUL, $1, $3); }
    | term '/' factor
      { $$ = ast_binary_new(BIN_DIV, $1, $3); }
    | term '%' factor
      { $$ = ast_binary_new(BIN_MOD, $1, $3); }
    | factor
      { $$ = $1; }
    ;

factor
    : '-' factor %prec UMINUS
      { $$ = ast_unary_new(UN_NEGATE, $2); }
    | '!' factor
      { $$ = ast_unary_new(UN_NOT, $2); }
    | primary
      { $$ = $1; }
    ;

primary
    : T_NUMBER
      { $$ = ast_number_new($1); }
    | T_IDENTIFIER
      { $$ = ast_identifier_new($1); }
    | '(' expression ')'
      { $$ = $2; }
    | sensor
      { $$ = $1; }
    | T_VERDADEIRO
      { $$ = ast_number_new(1.0); }
    | T_FALSO
      { $$ = ast_number_new(0.0); }
    ;

sensor
    : T_TEMPO
      { $$ = ast_sensor_new(SENSOR_TEMPO); }
    | T_JUROS
      { $$ = ast_sensor_new(SENSOR_JUROS); }
    ;

%%

void yyerror(const char *msg) {
    fprintf(stderr, "Erro de sintaxe na linha %d: %s\n", yylineno, msg);
}
