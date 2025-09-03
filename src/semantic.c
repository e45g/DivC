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
    // TODO: proper free
    free(table->current_scope);
    table->current_scope = s;
}

expr_type_t get_binary_result_type(expr_type_t left, expr_type_t right) {
    if(get_type_size(left) > get_type_size(right)) return left;
    return right;
}

void semantic_check_node(ast_node_t *node, symbol_table_t *table) {
    switch(node->type) {
        case AST_BINARY_OP: {
            semantic_check_node(node->expr.binary_op.left, table);
            semantic_check_node(node->expr.binary_op.right, table);
            expr_type_t left_t = node->expr.binary_op.left->resolved_type;
            expr_type_t right_t = node->expr.binary_op.right->resolved_type;
            node->resolved_type = get_binary_result_type(left_t, right_t);
            break;
        }
        case AST_NUMBER: {
            // TODO: make this smart by looking at the actual value
            node->resolved_type = INT32;
            break;
        }
        case AST_IDENTIFIER: {
            symbol_t *sym = map_get(table->current_scope, node->expr.identifier);
            if(sym == NULL) {
                show_error_unknown(node);
                break;
            }
            node->resolved_type = sym->type;
            break;
        }

        case AST_FUNCTION_CALL: {
            for(size_t i = 0; i < node->expr.call.arg_count; i++) {
                semantic_check_node(node->expr.call.args[i], table);
            }
            // TODO: add return type
            break;
        }

        default: {
            node->resolved_type = UNKNOWN_TYPE;
            break;
        }
    }
}

// TODO : lookup_sym function to check across scopes
void semantic_check_statement(ast_statement_t *stmt, symbol_table_t *table) {
    switch(stmt->type) {
        case AST_VAR_DECLARATION: {
            char *id = stmt->statement.declaration.identifier;
            if(map_get(table->current_scope, id) != NULL) {
                show_warning_redeclaration(stmt, id);
            } else {
                symbol_t *sym = malloc(sizeof(symbol_t));
                sym->kind = SYMBOL_VAR;
                sym->identifier = id;
                sym->scope_level = table->current_scope->level;
                sym->type = stmt->statement.declaration.t;
                map_add(table->current_scope, id, sym);
            }
            if(stmt->statement.declaration.initializer != NULL) semantic_check_statement(stmt->statement.declaration.initializer, table);
            break;
        }

        case AST_FUNC_DECLARATION: {
            char *id = stmt->statement.function.identifier;

            if(map_get(table->current_scope, id) != NULL) {
                show_warning_redeclaration(stmt, stmt->statement.function.identifier);
            }
            else {
                symbol_t *sym = malloc(sizeof(symbol_t));
                sym->kind = SYMBOL_FUNC;
                sym->identifier = id;
                sym->scope_level = table->current_scope->level;
                sym->type = stmt->statement.function.type;
                map_add(table->current_scope, id, sym);
            }

            enter_scope(table);

            struct arg *args = stmt->statement.function.args;
            for(size_t i = 0; i < stmt->statement.function.arg_count; i++) {
                symbol_t *sym = malloc(sizeof(symbol_t));
                sym->kind = SYMBOL_VAR;
                sym->identifier = args[i].identifier;
                sym->scope_level = table->current_scope->level;
                sym->type = args[i].type;

                map_add(table->current_scope, args[i].identifier, sym);

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
            symbol_t *sym = map_get(table->current_scope, stmt->statement.assignment.identifier);
            if(sym != NULL) {
                stmt->statement.assignment.resolved_var_type = sym->type;
            }
            else {
                stmt->statement.assignment.resolved_var_type = UNKNOWN_TYPE;
                fprintf(stderr, "Semantic error: Variable '%s' not found on line %d:%d", stmt->statement.assignment.identifier, stmt->pos.line, stmt->pos.column);
            }
            semantic_check_node(stmt->statement.assignment.value, table);
            break;
        }

        case AST_RETURN_STMT: {
            semantic_check_node(stmt->statement.ret.value, table);
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
