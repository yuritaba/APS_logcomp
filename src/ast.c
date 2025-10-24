#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

static void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static void ensure_stmt_capacity(ASTStmtList *list) {
    if (list->capacity == 0) {
        list->capacity = 8;
        list->items = xmalloc(list->capacity * sizeof(ASTStmt *));
    } else if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, list->capacity * sizeof(ASTStmt *));
        if (!list->items) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

static void ensure_print_arg_capacity(ASTPrintArgList *list) {
    if (list->capacity == 0) {
        list->capacity = 4;
        list->items = xmalloc(list->capacity * sizeof(ASTPrintArg *));
    } else if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, list->capacity * sizeof(ASTPrintArg *));
        if (!list->items) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

ASTProgram *ast_program_new(ASTStmtList *statements) {
    ASTProgram *program = xmalloc(sizeof(ASTProgram));
    program->statements = statements;
    return program;
}

ASTStmtList *ast_stmt_list_new(void) {
    ASTStmtList *list = xmalloc(sizeof(ASTStmtList));
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    return list;
}

void ast_stmt_list_append(ASTStmtList *list, ASTStmt *stmt) {
    ensure_stmt_capacity(list);
    list->items[list->count++] = stmt;
}

static ASTStmt *alloc_stmt(ASTStmtType type) {
    ASTStmt *stmt = xmalloc(sizeof(ASTStmt));
    stmt->type = type;
    return stmt;
}

static ASTExpr *alloc_expr(ASTExprType type) {
    ASTExpr *expr = xmalloc(sizeof(ASTExpr));
    expr->type = type;
    return expr;
}

ASTStmt *ast_var_decl_new(char *identifier, ASTExpr *expression) {
    ASTStmt *stmt = alloc_stmt(STMT_VAR_DECL);
    stmt->as.var_decl.identifier = identifier;
    stmt->as.var_decl.expression = expression;
    return stmt;
}

ASTStmt *ast_assignment_new(char *identifier, ASTExpr *expression) {
    ASTStmt *stmt = alloc_stmt(STMT_ASSIGNMENT);
    stmt->as.assignment.identifier = identifier;
    stmt->as.assignment.expression = expression;
    return stmt;
}

ASTStmt *ast_if_new(ASTExpr *condition, ASTStmtList *then_branch, ASTStmtList *else_branch) {
    ASTStmt *stmt = alloc_stmt(STMT_IF);
    stmt->as.if_stmt.condition = condition;
    stmt->as.if_stmt.then_branch = then_branch;
    stmt->as.if_stmt.else_branch = else_branch;
    return stmt;
}

ASTStmt *ast_while_new(ASTExpr *condition, ASTStmtList *body) {
    ASTStmt *stmt = alloc_stmt(STMT_WHILE);
    stmt->as.while_stmt.condition = condition;
    stmt->as.while_stmt.body = body;
    return stmt;
}

ASTStmt *ast_command_deposit_new(char *account, ASTExpr *amount) {
    ASTStmt *stmt = alloc_stmt(STMT_COMMAND);
    stmt->as.command.cmd_type = CMD_DEPOSIT;
    stmt->as.command.data.deposit.account = account;
    stmt->as.command.data.deposit.amount = amount;
    return stmt;
}

ASTStmt *ast_command_withdraw_new(char *account, ASTExpr *amount) {
    ASTStmt *stmt = alloc_stmt(STMT_COMMAND);
    stmt->as.command.cmd_type = CMD_WITHDRAW;
    stmt->as.command.data.withdraw.account = account;
    stmt->as.command.data.withdraw.amount = amount;
    return stmt;
}

ASTStmt *ast_command_transfer_new(char *from_account, char *to_account, ASTExpr *amount) {
    ASTStmt *stmt = alloc_stmt(STMT_COMMAND);
    stmt->as.command.cmd_type = CMD_TRANSFER;
    stmt->as.command.data.transfer.from_account = from_account;
    stmt->as.command.data.transfer.to_account = to_account;
    stmt->as.command.data.transfer.amount = amount;
    return stmt;
}

ASTStmt *ast_command_interest_new(char *account, ASTExpr *rate) {
    ASTStmt *stmt = alloc_stmt(STMT_COMMAND);
    stmt->as.command.cmd_type = CMD_INTEREST;
    stmt->as.command.data.interest.account = account;
    stmt->as.command.data.interest.rate = rate;
    return stmt;
}

ASTStmt *ast_command_print_new(ASTPrintArgList *args) {
    ASTStmt *stmt = alloc_stmt(STMT_COMMAND);
    stmt->as.command.cmd_type = CMD_PRINT;
    stmt->as.command.data.print_cmd.args = args;
    return stmt;
}

ASTExpr *ast_number_new(double value) {
    ASTExpr *expr = alloc_expr(EXPR_NUMBER);
    expr->as.number = value;
    return expr;
}

ASTExpr *ast_identifier_new(char *name) {
    ASTExpr *expr = alloc_expr(EXPR_IDENTIFIER);
    expr->as.identifier = name;
    return expr;
}

ASTExpr *ast_binary_new(ASTBinaryOp op, ASTExpr *left, ASTExpr *right) {
    ASTExpr *expr = alloc_expr(EXPR_BINARY);
    expr->as.binary.op = op;
    expr->as.binary.left = left;
    expr->as.binary.right = right;
    return expr;
}

ASTExpr *ast_unary_new(ASTUnaryOp op, ASTExpr *operand) {
    ASTExpr *expr = alloc_expr(EXPR_UNARY);
    expr->as.unary.op = op;
    expr->as.unary.operand = operand;
    return expr;
}

ASTExpr *ast_sensor_new(ASTSensorType sensor) {
    ASTExpr *expr = alloc_expr(EXPR_SENSOR);
    expr->as.sensor.sensor = sensor;
    return expr;
}

ASTPrintArgList *ast_print_arg_list_new(void) {
    ASTPrintArgList *list = xmalloc(sizeof(ASTPrintArgList));
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    return list;
}

void ast_print_arg_list_append(ASTPrintArgList *list, ASTPrintArg *arg) {
    ensure_print_arg_capacity(list);
    list->items[list->count++] = arg;
}

ASTPrintArg *ast_print_arg_expr_new(ASTExpr *expression) {
    ASTPrintArg *arg = xmalloc(sizeof(ASTPrintArg));
    arg->is_string = false;
    arg->value.expression = expression;
    return arg;
}

ASTPrintArg *ast_print_arg_string_new(char *value) {
    ASTPrintArg *arg = xmalloc(sizeof(ASTPrintArg));
    arg->is_string = true;
    arg->value.string_value = value;
    return arg;
}

static void free_expression(ASTExpr *expr);
static void free_statement(ASTStmt *stmt);
static void free_statement_list(ASTStmtList *list);
static void free_print_arg_list(ASTPrintArgList *list);

static void free_expression(ASTExpr *expr) {
    if (!expr) {
        return;
    }
    switch (expr->type) {
        case EXPR_NUMBER:
            break;
        case EXPR_IDENTIFIER:
            free(expr->as.identifier);
            break;
        case EXPR_BINARY:
            free_expression(expr->as.binary.left);
            free_expression(expr->as.binary.right);
            break;
        case EXPR_UNARY:
            free_expression(expr->as.unary.operand);
            break;
        case EXPR_SENSOR:
            break;
    }
    free(expr);
}

static void free_print_arg(ASTPrintArg *arg) {
    if (!arg) {
        return;
    }
    if (arg->is_string) {
        free(arg->value.string_value);
    } else {
        free_expression(arg->value.expression);
    }
    free(arg);
}

static void free_print_arg_list(ASTPrintArgList *list) {
    if (!list) {
        return;
    }
    for (size_t i = 0; i < list->count; ++i) {
        free_print_arg(list->items[i]);
    }
    free(list->items);
    free(list);
}

static void free_statement(ASTStmt *stmt) {
    if (!stmt) {
        return;
    }
    switch (stmt->type) {
        case STMT_VAR_DECL:
            free(stmt->as.var_decl.identifier);
            free_expression(stmt->as.var_decl.expression);
            break;
        case STMT_ASSIGNMENT:
            free(stmt->as.assignment.identifier);
            free_expression(stmt->as.assignment.expression);
            break;
        case STMT_IF:
            free_expression(stmt->as.if_stmt.condition);
            free_statement_list(stmt->as.if_stmt.then_branch);
            free_statement_list(stmt->as.if_stmt.else_branch);
            break;
        case STMT_WHILE:
            free_expression(stmt->as.while_stmt.condition);
            free_statement_list(stmt->as.while_stmt.body);
            break;
        case STMT_COMMAND:
            switch (stmt->as.command.cmd_type) {
                case CMD_DEPOSIT:
                    free(stmt->as.command.data.deposit.account);
                    free_expression(stmt->as.command.data.deposit.amount);
                    break;
                case CMD_WITHDRAW:
                    free(stmt->as.command.data.withdraw.account);
                    free_expression(stmt->as.command.data.withdraw.amount);
                    break;
                case CMD_TRANSFER:
                    free(stmt->as.command.data.transfer.from_account);
                    free(stmt->as.command.data.transfer.to_account);
                    free_expression(stmt->as.command.data.transfer.amount);
                    break;
                case CMD_INTEREST:
                    free(stmt->as.command.data.interest.account);
                    free_expression(stmt->as.command.data.interest.rate);
                    break;
                case CMD_PRINT:
                    free_print_arg_list(stmt->as.command.data.print_cmd.args);
                    break;
            }
            break;
    }
    free(stmt);
}

static void free_statement_list(ASTStmtList *list) {
    if (!list) {
        return;
    }
    for (size_t i = 0; i < list->count; ++i) {
        free_statement(list->items[i]);
    }
    free(list->items);
    free(list);
}

void ast_free_program(ASTProgram *program) {
    if (!program) {
        return;
    }
    free_statement_list(program->statements);
    free(program);
}
