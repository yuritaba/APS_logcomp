#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"

/* Generates BankVM assembly for the given AST program. Returns 0 on success. */
int generate_assembly(ASTProgram *program, FILE *out);

#endif /* CODEGEN_H */
