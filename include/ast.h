#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ASTProgram ASTProgram;
typedef struct ASTStmt ASTStmt;
typedef struct ASTExpr ASTExpr;
typedef struct ASTStmtList ASTStmtList;
typedef struct ASTPrintArg ASTPrintArg;
typedef struct ASTPrintArgList ASTPrintArgList;

typedef enum {
    STMT_VAR_DECL,
    STMT_ASSIGNMENT,
    STMT_IF,
    STMT_WHILE,
    STMT_COMMAND
} ASTStmtType;

typedef enum {
    CMD_DEPOSIT,
    CMD_WITHDRAW,
    CMD_TRANSFER,
    CMD_INTEREST,
    CMD_PRINT
} ASTCommandType;

typedef enum {
    EXPR_NUMBER,
    EXPR_IDENTIFIER,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_SENSOR
} ASTExprType;

typedef enum {
    SENSOR_TEMPO,
    SENSOR_JUROS
} ASTSensorType;

typedef enum {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_MOD,
    BIN_EQ,
    BIN_NEQ,
    BIN_LT,
    BIN_GT,
    BIN_LE,
    BIN_GE
} ASTBinaryOp;

typedef enum {
    UN_NEGATE,
    UN_NOT
} ASTUnaryOp;

struct ASTStmtList {
    ASTStmt **items;
    size_t count;
    size_t capacity;
};

struct ASTPrintArgList {
    ASTPrintArg **items;
    size_t count;
    size_t capacity;
};

struct ASTProgram {
    ASTStmtList *statements;
};

struct ASTStmt {
    ASTStmtType type;
    union {
        struct {
            char *identifier;
            ASTExpr *expression;
        } var_decl;
        struct {
            char *identifier;
            ASTExpr *expression;
        } assignment;
        struct {
            ASTExpr *condition;
            ASTStmtList *then_branch;
            ASTStmtList *else_branch; /* may be NULL */
        } if_stmt;
        struct {
            ASTExpr *condition;
            ASTStmtList *body;
        } while_stmt;
        struct {
            ASTCommandType cmd_type;
            union {
                struct {
                    char *account;
                    ASTExpr *amount;
                } deposit;
                struct {
                    char *account;
                    ASTExpr *amount;
                } withdraw;
                struct {
                    char *from_account;
                    char *to_account;
                    ASTExpr *amount;
                } transfer;
                struct {
                    char *account;
                    ASTExpr *rate;
                } interest;
                struct {
                    ASTPrintArgList *args;
                } print_cmd;
            } data;
        } command;
    } as;
};

struct ASTExpr {
    ASTExprType type;
    union {
        double number;
        char *identifier;
        struct {
            ASTBinaryOp op;
            ASTExpr *left;
            ASTExpr *right;
        } binary;
        struct {
            ASTUnaryOp op;
            ASTExpr *operand;
        } unary;
        struct {
            ASTSensorType sensor;
        } sensor;
    } as;
};

struct ASTPrintArg {
    bool is_string;
    union {
        char *string_value;
        ASTExpr *expression;
    } value;
};

ASTProgram *ast_program_new(ASTStmtList *statements);
ASTStmtList *ast_stmt_list_new(void);
void ast_stmt_list_append(ASTStmtList *list, ASTStmt *stmt);

ASTStmt *ast_var_decl_new(char *identifier, ASTExpr *expression);
ASTStmt *ast_assignment_new(char *identifier, ASTExpr *expression);
ASTStmt *ast_if_new(ASTExpr *condition, ASTStmtList *then_branch, ASTStmtList *else_branch);
ASTStmt *ast_while_new(ASTExpr *condition, ASTStmtList *body);
ASTStmt *ast_command_deposit_new(char *account, ASTExpr *amount);
ASTStmt *ast_command_withdraw_new(char *account, ASTExpr *amount);
ASTStmt *ast_command_transfer_new(char *from_account, char *to_account, ASTExpr *amount);
ASTStmt *ast_command_interest_new(char *account, ASTExpr *rate);
ASTStmt *ast_command_print_new(ASTPrintArgList *args);

ASTExpr *ast_number_new(double value);
ASTExpr *ast_identifier_new(char *name);
ASTExpr *ast_binary_new(ASTBinaryOp op, ASTExpr *left, ASTExpr *right);
ASTExpr *ast_unary_new(ASTUnaryOp op, ASTExpr *operand);
ASTExpr *ast_sensor_new(ASTSensorType sensor);

ASTPrintArgList *ast_print_arg_list_new(void);
void ast_print_arg_list_append(ASTPrintArgList *list, ASTPrintArg *arg);
ASTPrintArg *ast_print_arg_expr_new(ASTExpr *expression);
ASTPrintArg *ast_print_arg_string_new(char *value);

void ast_free_program(ASTProgram *program);

#endif /* AST_H */
