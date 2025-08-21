#include <stdio.h>

#include "semantic.h"
#include "hashmap.h"
#include "parser.h"

int semantic_check_statement(ast_statement_t *stmt, map_t *symbols) {
    switch(stmt->type) {
        case AST_VAR_DECLARATION: {
            char *id = stmt->statement.declaration.identifier;
            symbol_t sym = {
                .identifier = id,
                .kind = SYM_VAR,
                .type = stmt->statement.declaration.t,
            };
            if(map_add(symbols, id, &sym) == -1) {
                fprintf(stderr, "Variable '%s' redeclaration\n", id);
            }
        }
        default: {

        }
    }
    return 0;
}

void semantic_check(struct statement_list *ast) {
    map_t *symbols = get_map();
    struct statement_list *current = ast;
    while (current != NULL) {
        semantic_check_statement(current->statement, symbols);
        current = current->next;
    }
}
