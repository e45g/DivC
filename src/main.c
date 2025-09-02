#include "ir.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>

void print_ast(struct statement_list *statements);
void print_ast_types(struct statement_list *statements);
void print_tokens(token_t *token);
void print_ir(ir_instruction_list_t *list);
void generate_x64_code(ir_instruction_list_t *inst, FILE *f);

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if(!f) {
        printf("Failed to open specified file.\n");
        return 1;
    }

    size_t size = 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = (char *) malloc(size+1);
    fread(buffer, size, 1, f);
    buffer[size] = '\0';

    fclose(f);

    token_t *token = lexer_parse(buffer);
    // print_tokens(token);

    struct statement_list *statement = ast_parse(token);
    // print_ast(statement);

    semantic_check(statement);

    ir_instruction_list_t *ir = generate_ir(statement);
    // print_ir(ir);

    FILE *output_f = fopen("out.s", "w");
    generate_x64_code(ir, output_f);
    fclose(output_f);

    return 0;
}
