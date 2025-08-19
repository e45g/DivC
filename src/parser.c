#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "lexer.h"

struct statement_list *ast_parse(token_t *list) {
    token_t *current = list->next;

    struct statement_list *statements = malloc(sizeof(struct statement_list));
    struct statement_list *statements_head = statements;

    while(current != NULL) {
        statements->statement = ast_statement(&current);

        if(current != NULL) {
            statements->next = malloc(sizeof(struct statement_list));
            statements = statements->next;
        } else {
            statements->next = NULL;
        }
    }

    return statements_head;
}

int expect(token_t **token, token_type_t type) {
    if (*token == NULL) return -1;
    return (*token)->type == type ? 1 : 0;
}

int expect_move(token_t **token, token_type_t type) {
    if (expect(token, type) == 1) {
        *token = (*token)->next; 
        return 1;
    }
    return -1;
}

void next(token_t **token) {
    if(*token != NULL) {
        *token = (*token)->next;
    }
}


ast_node_t *factor(token_t **token) {
    switch((*token)->type) {
        case NUMBER: {
            ast_node_t *node = malloc(sizeof(ast_node_t));
            node->type = AST_NUMBER;
            node->expr.integer = atoi((*token)->value);
            next(token);
            return node;
        }
        default: {
            return NULL;
        }
    }
    return NULL;
}

ast_node_t *term(token_t **token) {
    ast_node_t *f = factor(token);
    while(expect(token, STAR) || expect(token, SLASH)) {
        ast_node_t *node = malloc(sizeof(ast_node_t));
        node->type=AST_BINARY_OP;
        node->expr.binary_op.left = f;
        node->expr.binary_op.op = (*token)->type;
        next(token);
        node->expr.binary_op.right = factor(token);

        f = node;
    }

    return f;

}

ast_node_t *expression(token_t **token) {
    ast_node_t *t = term(token);

    while(expect(token, PLUS) || expect(token, MINUS)) {
        ast_node_t *node = malloc(sizeof(ast_node_t));
        node->type=AST_BINARY_OP;
        node->expr.binary_op.left = t;
        node->expr.binary_op.op = (*token)->type;
        next(token);
        node->expr.binary_op.right = term(token);

        t = node;
    }

    return t;
}


ast_statement_t *ast_variable(token_t **token, expr_type_t type, char *name) {
    ast_statement_t *var = malloc(sizeof(ast_statement_t));

    if(expect_move(token, ASSIGN) == 1) {
        var->type = AST_DECLARATION;
        var->statement.declaration.identifier = name;
        var->statement.declaration.t = type;

        ast_statement_t *assignment = malloc(sizeof(ast_statement_t));
        assignment->type = AST_ASSIGNMENT;
        assignment->statement.assignment.identifier = name;
        assignment->statement.assignment.value = expression(token);

        var->statement.declaration.initializer = assignment;
    }

    if(!expect(token, SEMICOLON)) {
        printf("Missing semicolon;\n");
    } else {
        next(token);
    }

    return var;
}

ast_statement_t *ast_statement(token_t **token) {
    switch((*token)->type) {
        case I8:
        case I16:
        case I32:
        case I64:
        case U8:
        case U16:
        case U32:
        case U64:
        case SHORT:
        case LONG:
        case UNSIGNED: {
            expr_type_t type = ast_type(token);

            char *name = (*token)->value;
            if (expect_move(token, IDENTIFIER) == 1) {
                if (expect(token, ASSIGN) == 1) {
                    return ast_variable(token, type, name);
                }
            }
            break;
        }
        default: {
            printf("Unexpected token type: %d\n", (*token)->type);
            next(token);
            return NULL;
        }
    }
    return NULL;
}


expr_type_t ast_type(token_t **token) {
    if (*token == NULL) return UNKNOWN_TYPE;

    switch((*token)->type) {
        case LONG:
            next(token);
            if (expect_move(token, I32)) return INT64;
            break;
        case SHORT:
            next(token);
            if (expect_move(token, I32)) return INT16;
            break;
        case UNSIGNED: {
            next(token);
            expr_type_t t = ast_type(token); // recursively get the base type
            switch(t) {
                case INT8: return UINT8;
                case INT16: return UINT16;
                case INT32: return UINT32;
                case INT64: return UINT64;
                default: printf("Unexpected after unsigned\n"); return UNKNOWN_TYPE;
            }
        }
        case I8: next(token); return INT8;
        case I16: next(token); return INT16;
        case I32: next(token); return INT32;
        case I64: next(token); return INT64;
        default: printf("Unknown type\n"); return UNKNOWN_TYPE;
    }
    return UNKNOWN_TYPE;
}

