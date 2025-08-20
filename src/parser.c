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
        case IDENTIFIER: {
            ast_node_t *node = malloc(sizeof(ast_node_t));
            node->type = AST_IDENTIFIER;
            node->expr.identifier = (*token)->value;
            next(token);
            return node;
        }
        case LEFT_PAREN: {
            next(token);
            ast_node_t *node = expression(token);

            if (!expect_move(token, RIGHT_PAREN)) {
                printf("Error: expected ')'\n");
                return NULL;
            }

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


ast_statement_t *ast_var_declaration(token_t **token, expr_type_t type, char *name) {
    ast_statement_t *var = malloc(sizeof(ast_statement_t));

    if(expect_move(token, ASSIGN) == 1) {
        var->type = AST_VAR_DECLARATION;
        var->statement.declaration.identifier = name;
        var->statement.declaration.t = type;

        ast_statement_t *assignment = malloc(sizeof(ast_statement_t));
        assignment->type = AST_VAR_ASSIGNMENT;
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

ast_statement_t *ast_var_assignment(token_t **token, char *name) {
    ast_statement_t *var = malloc(sizeof(ast_statement_t));

    if(expect_move(token, ASSIGN) == 1) {
        var->type = AST_VAR_ASSIGNMENT;
        var->statement.assignment.identifier = name;
        var->statement.assignment.value = expression(token);
    }

    if(!expect(token, SEMICOLON)) {
        printf("Missing semicolon;\n");
    } else {
        next(token);
    }

    return var;
}

ast_statement_t *ast_return(token_t **token) {
    ast_statement_t *node = malloc(sizeof(ast_statement_t));

    node->type = AST_RETURN_STMT;
    node->statement.ret.value = expression(token);

    if(!expect(token, SEMICOLON)) {

        printf("Missing semicolon;\n");
    } else {
        next(token);
    }

    return node;
}

struct block_member *ast_block(token_t **token) {
    if(expect_move(token, LEFT_CURLY) < 0) {
        printf("Expected {, found: %s\n", (*token)->value);
        return NULL;
    }

    if(expect_move(token, RIGHT_CURLY) == 1){
        return NULL;
    } 

    struct block_member *block = malloc(sizeof(struct block_member));     
    struct block_member *head = block;
    head->stack_size = 0;

    while(expect_move(token, RIGHT_CURLY) < 0) {
        ast_statement_t *statement = ast_statement(token);

        if(statement->type == AST_VAR_DECLARATION) head->stack_size += get_type_size(statement->statement.declaration.t);

        block->value = statement;
        block->next = malloc(sizeof(struct block_member));     
        block = block->next;
        block->value = 0;
    }
    next(token);
    return head;
}

ast_statement_t *ast_function(token_t **token, expr_type_t type, char *name) {
    ast_statement_t *func = malloc(sizeof(ast_statement_t));

    func->type = AST_FUNC_DECLARATION;
    func->statement.function.type = type;
    func->statement.function.identifier = name;
    next(token);
    next(token);
    func->statement.function.block = ast_block(token);

    return func;
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
                    return ast_var_declaration(token, type, name);
                }

                else if (expect(token, LEFT_PAREN) == 1) {
                    return ast_function(token, type, name);
                }
            }
            break;
        }
        case IDENTIFIER: {
            char *name = (*token)->value; 
            next(token);
            if(expect(token, ASSIGN) == 1) {
                return ast_var_assignment(token, name);
            }
            break;
        }

        case RETURN: {
            next(token);
            return ast_return(token);
        }
        default: {
            printf("Unexpected token type: %d, value: %s\n", (*token)->type, (*token)->value);
            next(token);
            return NULL;
        }
    }
    return NULL;
}

expr_type_t ast_type(token_t **token) {
    if (*token == NULL) return UNKNOWN_TYPE;

    expr_type_t res;

    switch((*token)->type) {
        case LONG:
            next(token);
            if (expect_move(token, I32)) res = INT64;
            break;
        case SHORT:
            next(token);
            if (expect_move(token, I32)) res = INT16;
            break;
        case UNSIGNED: {
            next(token);
            expr_type_t t = ast_type(token);
            switch(t) {
                case INT8:
                    res = UINT8;
                    break;
                case INT16:
                    res = UINT16;
                    break;
                case INT32:
                    res = UINT32;
                    break;
                case INT64:
                    res = UINT64;
                    break;
                default: 
                    printf("Unexpected after unsigned\n"); 
                    res =  UNKNOWN_TYPE;
                    break;
            }
            break;
        }
        case I8: {
            next(token);
            res = INT8;
            break;
        }
        case I16: {
            next(token);
            res = INT16;
            break;
        }
        case I32: {
            next(token);
            res = INT32;
            break;
        }
        case I64: {
            next(token);
            res = INT64;
            break;
        }

        case U8: {
            next(token);
            res = UINT8;
            break;
        }
        case U16: {
            next(token);
            res = UINT16;
            break;
        }
        case U32: {
            next(token);
            res = UINT32;
            break;
        }
        case U64: {
            next(token);
            res = UINT64;
            break;
        }
        default: printf("Unknown type\n"); return UNKNOWN_TYPE;
    }

    if(expect_move(token, STAR)) {
        res |= TYPE_POINTER;
    }

    return res;
}

int get_type_size(expr_type_t type) {
    // TODO: check for pointer

    switch(type) {
        case INT8: return 1;        
        case INT16: return 2;        
        case INT32: return 4;        
        case INT64: return 8;        
        case UINT8: return 1;        
        case UINT16: return 2;        
        case UINT32: return 4;        
        case UINT64: return 8;        
        case F32: return 4;
        case F64: return 8;
        case VOID_T: return 0;
        case STRUC: return 0;
        default: return -1;
    }
} 
