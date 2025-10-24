#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

static char *xstrdup(const char *src) {
    if (!src) {
        return NULL;
    }
    size_t len = strlen(src);
    char *copy = malloc(len + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, src, len + 1);
    return copy;
}

typedef struct {
    char *name;
    bool is_account;
} Symbol;

typedef struct {
    Symbol *items;
    size_t count;
    size_t capacity;
} SymbolTable;

typedef struct {
    FILE *out;
    int label_counter;
    bool has_error;
    SymbolTable symbols;
} CodegenContext;

static void symbol_table_init(SymbolTable *table) {
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

static void symbol_table_free(SymbolTable *table) {
    for (size_t i = 0; i < table->count; ++i) {
        free(table->items[i].name);
    }
    free(table->items);
}

static Symbol *symbol_table_find(SymbolTable *table, const char *name) {
    for (size_t i = 0; i < table->count; ++i) {
        if (strcmp(table->items[i].name, name) == 0) {
            return &table->items[i];
        }
    }
    return NULL;
}

static Symbol *symbol_table_add(SymbolTable *table, const char *name, bool is_account) {
    if (table->capacity == table->count) {
        size_t new_cap = table->capacity == 0 ? 8 : table->capacity * 2;
        Symbol *new_items = realloc(table->items, new_cap * sizeof(Symbol));
        if (!new_items) {
            return NULL;
        }
        table->items = new_items;
        table->capacity = new_cap;
    }
    char *copy = xstrdup(name);
    if (!copy) {
        return NULL;
    }
    table->items[table->count].name = copy;
    table->items[table->count].is_account = is_account;
    table->count++;
    return &table->items[table->count - 1];
}

static void codegen_error(CodegenContext *ctx, const char *fmt, ...) {
    if (ctx->has_error) {
        return;
    }
    ctx->has_error = true;
    fprintf(stderr, "Code generation error: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static char *create_label(CodegenContext *ctx, const char *prefix) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s_%d", prefix, ctx->label_counter++);
    char *label = xstrdup(buffer);
    return label;
}

static void emit_expression(CodegenContext *ctx, ASTExpr *expr);
static void emit_statement(CodegenContext *ctx, ASTStmt *stmt);
static void emit_statement_list(CodegenContext *ctx, ASTStmtList *list);
static void emit_print_args(CodegenContext *ctx, ASTPrintArgList *args);

static void emit_line(CodegenContext *ctx, const char *fmt, ...) {
    if (ctx->has_error) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);
    fputc('\n', ctx->out);
}

static void ensure_symbol(CodegenContext *ctx, const char *name, bool is_account) {
    Symbol *symbol = symbol_table_find(&ctx->symbols, name);
    if (!symbol) {
        symbol = symbol_table_add(&ctx->symbols, name, is_account);
        if (!symbol) {
            codegen_error(ctx, "falha ao registrar símbolo '%s'", name);
            return;
        }
    } else if (is_account && !symbol->is_account) {
        symbol->is_account = true;
    }
}

static bool ensure_account(CodegenContext *ctx, const char *name) {
    Symbol *symbol = symbol_table_find(&ctx->symbols, name);
    if (!symbol || !symbol->is_account) {
        codegen_error(ctx, "identificador '%s' não é uma conta declarada", name);
        return false;
    }
    return true;
}

static void emit_var_decl(CodegenContext *ctx, ASTStmt *stmt) {
    const char *name = stmt->as.var_decl.identifier;
    if (symbol_table_find(&ctx->symbols, name)) {
        codegen_error(ctx, "conta '%s' já declarada", name);
        return;
    }
    ensure_symbol(ctx, name, true);
    emit_line(ctx, "ACCOUNT_INIT %s", name);
    emit_expression(ctx, stmt->as.var_decl.expression);
    emit_line(ctx, "STORE %s", name);
}

static void emit_assignment(CodegenContext *ctx, ASTStmt *stmt) {
    const char *name = stmt->as.assignment.identifier;
    ensure_symbol(ctx, name, false);
    emit_expression(ctx, stmt->as.assignment.expression);
    emit_line(ctx, "STORE %s", name);
}

static void emit_if(CodegenContext *ctx, ASTStmt *stmt) {
    char *else_label = create_label(ctx, "else");
    char *end_label = create_label(ctx, "endif");
    emit_expression(ctx, stmt->as.if_stmt.condition);
    emit_line(ctx, "JMP_IF_FALSE %s", stmt->as.if_stmt.else_branch ? else_label : end_label);
    emit_statement_list(ctx, stmt->as.if_stmt.then_branch);
    if (stmt->as.if_stmt.else_branch) {
        emit_line(ctx, "JMP %s", end_label);
        emit_line(ctx, "LABEL %s", else_label);
        emit_statement_list(ctx, stmt->as.if_stmt.else_branch);
    }
    emit_line(ctx, "LABEL %s", end_label);
    free(else_label);
    free(end_label);
}

static void emit_while(CodegenContext *ctx, ASTStmt *stmt) {
    char *start_label = create_label(ctx, "loop");
    char *end_label = create_label(ctx, "endloop");
    emit_line(ctx, "LABEL %s", start_label);
    emit_expression(ctx, stmt->as.while_stmt.condition);
    emit_line(ctx, "JMP_IF_FALSE %s", end_label);
    emit_statement_list(ctx, stmt->as.while_stmt.body);
    emit_line(ctx, "JMP %s", start_label);
    emit_line(ctx, "LABEL %s", end_label);
    free(start_label);
    free(end_label);
}

static void emit_command(CodegenContext *ctx, ASTStmt *stmt) {
    switch (stmt->as.command.cmd_type) {
        case CMD_DEPOSIT:
            if (!ensure_account(ctx, stmt->as.command.data.deposit.account)) {
                return;
            }
            emit_expression(ctx, stmt->as.command.data.deposit.amount);
            emit_line(ctx, "DEPOSIT %s", stmt->as.command.data.deposit.account);
            break;
        case CMD_WITHDRAW:
            if (!ensure_account(ctx, stmt->as.command.data.withdraw.account)) {
                return;
            }
            emit_expression(ctx, stmt->as.command.data.withdraw.amount);
            emit_line(ctx, "WITHDRAW %s", stmt->as.command.data.withdraw.account);
            break;
        case CMD_TRANSFER:
            if (!ensure_account(ctx, stmt->as.command.data.transfer.from_account) ||
                !ensure_account(ctx, stmt->as.command.data.transfer.to_account)) {
                return;
            }
            emit_expression(ctx, stmt->as.command.data.transfer.amount);
            emit_line(ctx, "TRANSFER %s %s",
                      stmt->as.command.data.transfer.from_account,
                      stmt->as.command.data.transfer.to_account);
            break;
        case CMD_INTEREST:
            if (!ensure_account(ctx, stmt->as.command.data.interest.account)) {
                return;
            }
            emit_expression(ctx, stmt->as.command.data.interest.rate);
            emit_line(ctx, "APPLY_INTEREST %s", stmt->as.command.data.interest.account);
            break;
        case CMD_PRINT:
            emit_print_args(ctx, stmt->as.command.data.print_cmd.args);
            break;
    }
}

static void emit_print_args(CodegenContext *ctx, ASTPrintArgList *args) {
    for (size_t i = 0; i < args->count; ++i) {
        ASTPrintArg *arg = args->items[i];
        if (arg->is_string) {
            const char *src = arg->value.string_value;
            size_t len = strlen(src);
            fputs("PRINT_STR_LITERAL \"", ctx->out);
            for (size_t j = 0; j < len; ++j) {
                char c = src[j];
                switch (c) {
                    case '\\':
                        fputs("\\\\", ctx->out);
                        break;
                    case '"':
                        fputs("\\\"", ctx->out);
                        break;
                    case '\n':
                        fputs("\\n", ctx->out);
                        break;
                    case '\t':
                        fputs("\\t", ctx->out);
                        break;
                    default:
                        fputc(c, ctx->out);
                        break;
                }
            }
            fputs("\"\n", ctx->out);
        } else {
            emit_expression(ctx, arg->value.expression);
            emit_line(ctx, "PRINT");
        }
    }
}

static void emit_expression(CodegenContext *ctx, ASTExpr *expr) {
    if (ctx->has_error) {
        return;
    }
    switch (expr->type) {
        case EXPR_NUMBER:
            emit_line(ctx, "PUSH_CONST %.17g", expr->as.number);
            break;
        case EXPR_IDENTIFIER:
            ensure_symbol(ctx, expr->as.identifier, false);
            emit_line(ctx, "LOAD %s", expr->as.identifier);
            break;
        case EXPR_SENSOR:
            switch (expr->as.sensor.sensor) {
                case SENSOR_TEMPO:
                    emit_line(ctx, "SENSOR_TEMPO");
                    break;
                case SENSOR_JUROS:
                    emit_line(ctx, "SENSOR_JUROS");
                    break;
            }
            break;
        case EXPR_UNARY:
            emit_expression(ctx, expr->as.unary.operand);
            if (expr->as.unary.op == UN_NEGATE) {
                emit_line(ctx, "NEG");
            } else {
                emit_line(ctx, "NOT");
            }
            break;
        case EXPR_BINARY:
            emit_expression(ctx, expr->as.binary.left);
            emit_expression(ctx, expr->as.binary.right);
            switch (expr->as.binary.op) {
                case BIN_ADD:
                    emit_line(ctx, "ADD");
                    break;
                case BIN_SUB:
                    emit_line(ctx, "SUB");
                    break;
                case BIN_MUL:
                    emit_line(ctx, "MUL");
                    break;
                case BIN_DIV:
                    emit_line(ctx, "DIV");
                    break;
                case BIN_MOD:
                    emit_line(ctx, "MOD");
                    break;
                case BIN_EQ:
                    emit_line(ctx, "CMP_EQ");
                    break;
                case BIN_NEQ:
                    emit_line(ctx, "CMP_NE");
                    break;
                case BIN_LT:
                    emit_line(ctx, "CMP_LT");
                    break;
                case BIN_GT:
                    emit_line(ctx, "CMP_GT");
                    break;
                case BIN_LE:
                    emit_line(ctx, "CMP_LE");
                    break;
                case BIN_GE:
                    emit_line(ctx, "CMP_GE");
                    break;
            }
            break;
    }
}

static void emit_statement(CodegenContext *ctx, ASTStmt *stmt) {
    if (ctx->has_error) {
        return;
    }
    switch (stmt->type) {
        case STMT_VAR_DECL:
            emit_var_decl(ctx, stmt);
            break;
        case STMT_ASSIGNMENT:
            emit_assignment(ctx, stmt);
            break;
        case STMT_IF:
            emit_if(ctx, stmt);
            break;
        case STMT_WHILE:
            emit_while(ctx, stmt);
            break;
        case STMT_COMMAND:
            emit_command(ctx, stmt);
            break;
    }
}

static void emit_statement_list(CodegenContext *ctx, ASTStmtList *list) {
    if (!list) {
        return;
    }
    for (size_t i = 0; i < list->count; ++i) {
        emit_statement(ctx, list->items[i]);
    }
}

int generate_assembly(ASTProgram *program, FILE *out) {
    if (!program || !out) {
        return 1;
    }

    CodegenContext ctx = {
        .out = out,
        .label_counter = 0,
        .has_error = false,
    };
    symbol_table_init(&ctx.symbols);

    emit_line(&ctx, "# BankVM assembly generated by MoneyLang compiler");
    emit_statement_list(&ctx, program->statements);
    emit_line(&ctx, "HALT");

    symbol_table_free(&ctx.symbols);
    return ctx.has_error ? 1 : 0;
}
