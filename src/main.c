#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>

void print_ast(struct statement_list *statements);
void print_ast_types(struct statement_list *statements);
void print_tokens(token_t *token);

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


    token_t *token = lexer_parse(buffer);
    // print_tokens(token);

    struct statement_list *statement = ast_parse(token);
    // print_ast(statement);

    semantic_check(statement);

    return 0;
}
