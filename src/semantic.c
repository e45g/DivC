#include <stdio.h>
#include <stdlib.h>

#include "semantic.h"
#include "hashmap.h"
#include "lexer.h"
#include "parser.h"

void show_warning_redeclaration(ast_statement_t *stmt, char *id) {
  fprintf(stderr, "Semantic warning: Redefinition of '%s' on line %d:%d\n",
          id, stmt->pos.line, stmt->pos.column);
}

void show_error_unknown(ast_node_t *node) {
  fprintf(stderr, "Semantic error: Variable '%s' not found on line %d:%d\n",
          node->expr.identifier, node->pos.line, node->pos.column);
}

void enter_scope(symbol_table_t *table) {
    struct scope *s = calloc(1, sizeof(struct scope));
    s->level = table->current_scope->level+1;
    s->parent = table->current_scope;
    table->current_scope = s;
}

void exit_scope(symbol_table_t *table) {
    if(table->current_scope == table->global_scope) return;

    struct scope *s = table->current_scope->parent;
    free(table->current_scope);
    table->current_scope = s;
}

void semantic_check_node(ast_node_t *node, symbol_table_t *table) {
    switch(node->type) {
        case AST_BINARY_OP: {
            semantic_check_node(node->expr.binary_op.left, table);
            semantic_check_node(node->expr.binary_op.right, table);
            break;
        }
        case AST_NUMBER: {
            break;
        }
        case AST_IDENTIFIER: {
            if(map_get(table->current_scope, node->expr.identifier) == NULL) {
                show_error_unknown(node);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void semantic_check_statement(ast_statement_t *stmt, symbol_table_t *table) {
    switch(stmt->type) {
        case AST_VAR_DECLARATION: {
            char *id = stmt->statement.declaration.identifier;
            if(map_get(table->current_scope, id) != NULL) {
                show_warning_redeclaration(stmt, id);
            } else {
                symbol_t sym = {
                    .kind = SYMBOL_VAR,
                    .identifier = id,
                    .scope_level = table->current_scope->level,
                    .type = stmt->statement.declaration.t,
                };
                map_add(table->current_scope, id, &sym);
            }
            if(stmt->statement.declaration.initializer != NULL) semantic_check_statement(stmt->statement.declaration.initializer, table);
            break;
        }

        case AST_FUNC_DECLARATION: {
            if(map_get(table->current_scope, stmt->statement.function.identifier) != NULL) {
                show_warning_redeclaration(stmt, stmt->statement.function.identifier);
            }
            enter_scope(table);

            struct arg *args = stmt->statement.function.args;
            for(size_t i = 0; i < stmt->statement.function.arg_count; i++) {
                symbol_t sym = {
                    .kind = SYMBOL_VAR,
                    .identifier = args[i].identifier,
                    .scope_level = table->current_scope->level,
                    .type = args[i].type,
                };

                map_add(table->current_scope, args[i].identifier, &sym);
            }
            struct block_member *block = stmt->statement.function.block;
            while(block != NULL) {
                if (block->value != NULL) {
                    semantic_check_statement(block->value, table);
                }
                block = block->next;
            }
            exit_scope(table);
            break;
}

        case AST_VAR_ASSIGNMENT: {
            semantic_check_node(stmt->statement.assignment.value, table);
            break;
        }

        default: {
            break;
        }
    }
}

void semantic_check(struct statement_list *ast) {
    symbol_table_t table = {0};
    table.global_scope = calloc(1, sizeof(struct scope));
    table.global_scope->level = 0;
    table.global_scope->parent = NULL;
    table.current_scope = table.global_scope;

    struct statement_list *current = ast;
    while (current != NULL) {
        if(current->statement != NULL){
            semantic_check_statement(current->statement, &table);
        }
        current = current->next;
    }
}
