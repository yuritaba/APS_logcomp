#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "codegen.h"

extern int yyparse(void);
extern int yylex_destroy(void);
extern FILE *yyin;
extern ASTProgram *root_program;

static void print_usage(const char *program_name) {
    fprintf(stderr, "Uso: %s <arquivo.money> [-o saida.asm]\n", program_name);
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    const char *output_path = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: esperava caminho após '-o'.\n");
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            output_path = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Opção desconhecida: %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else if (!input_path) {
            input_path = argv[i];
        } else {
            fprintf(stderr, "Erro: múltiplos arquivos de entrada fornecidos.\n");
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (!input_path) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input_file = fopen(input_path, "r");
    if (!input_file) {
        perror("Não foi possível abrir arquivo de entrada");
        return EXIT_FAILURE;
    }

    yyin = input_file;

    if (yyparse() != 0 || !root_program) {
        fprintf(stderr, "Falha na análise do programa.\n");
        fclose(input_file);
        yylex_destroy();
        return EXIT_FAILURE;
    }

    FILE *output_file = stdout;
    if (output_path) {
        output_file = fopen(output_path, "w");
        if (!output_file) {
            perror("Não foi possível abrir arquivo de saída");
            fclose(input_file);
            ast_free_program(root_program);
            yylex_destroy();
            return EXIT_FAILURE;
        }
    }

    int result = generate_assembly(root_program, output_file);

    if (output_path) {
        fclose(output_file);
    }
    fclose(input_file);
    ast_free_program(root_program);
    yylex_destroy();

    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
