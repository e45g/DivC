#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "lexer.h"

#define show_error_msg(msg, ...) fprintf(stderr, "Syntax error: " msg "\n", __VA_ARGS__);

void show_error_expected(token_t *token, char *expected) {
    fprintf(stderr, "Syntax error: Expected '%s', found '%s' on line %d:%d\n", expected, token->value, token->pos.line, token->pos.column);
}

void show_error_unexpected(token_t *token) {
    fprintf(stderr, "Syntax error: Unexpected token '%s' on line %d:%d\n", token->value, token->pos.line, token->pos.column);
}

struct statement_list *ast_parse(token_t *list) {
    token_t *current = list->next;

    struct statement_list *statements = malloc(sizeof(struct statement_list));
    struct statement_list *statements_head = statements;

    while(current->type != TOKEN_EOF) {
        statements->statement = ast_statement(&current);

        statements->next = malloc(sizeof(struct statement_list));
        statements->next->statement = NULL;
        statements->next->next = NULL;
        statements = statements->next;
    }

    return statements_head;
}

int expect(token_t **token, token_type_t type) {
    if (*token == NULL) return -1;
    return (*token)->type == type ? 1 : 0;
}

int expect_move(token_t **token, token_type_t type) {
    if (expect(token, type)) {
        if((*token)->next != NULL) *token = (*token)->next;
        return 1;
    }
    return 0;
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
            node->pos = (*token)->pos;
            node->type = AST_NUMBER;
            node->expr.integer = atoi((*token)->value);
            next(token);
            return node;
        }
        case IDENTIFIER: {
            ast_node_t *node = malloc(sizeof(ast_node_t));
            node->pos = (*token)->pos;
            node->type = AST_IDENTIFIER;
            node->expr.identifier = (*token)->value;
            next(token);
            return node;
        }
        case LEFT_PAREN: {
            next(token);
            ast_node_t *node = expression(token);
            node->pos = (*token)->pos;

            if (!expect_move(token, RIGHT_PAREN)) {
                show_error_expected(*token, ")");
                return NULL;
            }
            return node;
        }
        default: {
            show_error_unexpected(*token);
            return NULL;
        }
    }
    return NULL;
}

ast_node_t *term(token_t **token) {
    ast_node_t *f = factor(token);
    if(f == NULL) return NULL;

    while(expect(token, STAR) || expect(token, SLASH)) {
        ast_node_t *node = malloc(sizeof(ast_node_t));
        node->pos = (*token)->pos;
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
    if(t == NULL) return NULL;

    while(expect(token, PLUS) || expect(token, MINUS)) {
        ast_node_t *node = malloc(sizeof(ast_node_t));

        node->pos = (*token)->pos;
        node->type=AST_BINARY_OP;
        node->expr.binary_op.left = t;
        node->expr.binary_op.op = (*token)->type;
        next(token);
        node->expr.binary_op.right = term(token);

        t = node;
    }

    return t;
}


ast_statement_t *ast_var_declaration(token_t **token, expr_type_t type, char *name, pos_t pos) {
    ast_statement_t *var = malloc(sizeof(ast_statement_t));

    var->pos = pos;
    var->type = AST_VAR_DECLARATION;
    var->statement.declaration.identifier = name;
    var->statement.declaration.t = type;

    if(expect_move(token, ASSIGN)) {
        ast_statement_t *assignment = malloc(sizeof(ast_statement_t));
        assignment->type = AST_VAR_ASSIGNMENT;
        assignment->statement.assignment.identifier = name;
        assignment->statement.assignment.value = expression(token);

        var->statement.declaration.initializer = assignment;
    }
    else if(expect(token, SEMICOLON)) {
        var->statement.declaration.initializer = NULL;
    }
    else {
        show_error_unexpected(*token);
    }

    if(!expect_move(token, SEMICOLON)) {
        show_error_expected(*token, ";");
    }

    return var;
}

ast_statement_t *ast_var_assignment(token_t **token, char *name, pos_t pos) {
    ast_statement_t *var = malloc(sizeof(ast_statement_t));
    var->pos = pos;

    if(expect_move(token, ASSIGN)) {
        var->type = AST_VAR_ASSIGNMENT;
        var->statement.assignment.identifier = name;
        var->statement.assignment.value = expression(token);
    }
    else {
        show_error_expected(*token, "=");
    }

    if(!expect(token, SEMICOLON)) {
        show_error_unexpected(*token);
    } else {
        next(token);
    }

    return var;
}

ast_statement_t *ast_return(token_t **token, pos_t pos) {
    ast_statement_t *node = malloc(sizeof(ast_statement_t));

    node->pos = pos;
    node->type = AST_RETURN_STMT;
    node->statement.ret.value = expression(token);

    if(!expect(token, SEMICOLON)) {
        show_error_unexpected(*token);
    } else {
        next(token);
    }

    return node;
}

struct block_member *ast_block(token_t **token) {
    if(!expect_move(token, LEFT_CURLY)) {
        show_error_expected(*token, "(");
        return NULL;
    }

    if(expect_move(token, RIGHT_CURLY)){
        return NULL;
    }

    struct block_member *block = malloc(sizeof(struct block_member));
    struct block_member *head = block;
    head->stack_size = 0;

    while(!expect_move(token, RIGHT_CURLY)) {
        ast_statement_t *statement = ast_statement(token);
        if(statement == NULL) continue;

        if(statement->type == AST_VAR_DECLARATION) head->stack_size += get_type_size(statement->statement.declaration.t);

        block->value = statement;
        block->next = malloc(sizeof(struct block_member));
        block = block->next;
        block->value = 0;
    }
    return head;
}

ast_statement_t *ast_function(token_t **token, expr_type_t type, char *name, pos_t pos) {
    ast_statement_t *func = malloc(sizeof(ast_statement_t));

    func->pos = pos;
    func->type = AST_FUNC_DECLARATION;
    func->statement.function.type = type;
    func->statement.function.identifier = name;

    expect_move(token, LEFT_PAREN);

    size_t arg_c = 0;
    struct arg *args = NULL;
    if(expect_move(token, RIGHT_PAREN)) {
        arg_c = 0;
    }
    else if (expect(token, VOID) && (*token)->next && (*token)->next->type == RIGHT_PAREN) {
        next(token);
        expect_move(token, RIGHT_PAREN);
        arg_c = 0;
        args = NULL;
    }
    else {
        do {
            expr_type_t type = ast_type(token);
            char *id = NULL;
            if(expect(token, IDENTIFIER)) {
                id = (*token)->value;
                next(token);
            }
            args = realloc(args, sizeof(struct arg) * (arg_c+1));
            args[arg_c].identifier = id;
            args[arg_c].type = type;
            arg_c++;
        } while(expect_move(token, COMMA));

        if(!expect_move(token, RIGHT_PAREN)) {
            show_error_expected(*token, ")");
        }
    }
    func->statement.function.args = args;
    func->statement.function.arg_count = arg_c;


    if(expect(token, LEFT_CURLY)) {
        func->statement.function.block = ast_block(token);
        if(func->statement.function.block != NULL) {
            // TODO: figure stack size properly (var after return counted);
            func->statement.function.stack_size = func->statement.function.block->stack_size;
        }
    }
    else {
        func->statement.function.block = NULL;
    }

    expect_move(token, SEMICOLON);

    return func;
}

ast_statement_t *ast_statement(token_t **token) {
    pos_t pos = (*token)->pos;
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

            if (!expect(token, IDENTIFIER)) {
                show_error_expected(*token, "IDENTIFIER");
                return NULL;
            }

            char *name = (*token)->value;

            if (expect_move(token, IDENTIFIER)) {
                if (expect(token, ASSIGN) || expect(token, SEMICOLON)) {
                    return ast_var_declaration(token, type, name, pos);
                }

                else if (expect(token, LEFT_PAREN)) {
                    return ast_function(token, type, name, pos);
                }
                else {
                    show_error_unexpected(*token);
                    next(token);
                    return NULL;
                }
            }
            break;
        }
        case IDENTIFIER: {
            char *name = (*token)->value;
            next(token);
            if(expect(token, ASSIGN)) {
                return ast_var_assignment(token, name, pos);
            }
            else {
                show_error_unexpected(*token);
                next(token);
                return NULL;
            }
            break;
        }

        case RETURN: {
            next(token);
            return ast_return(token, pos);
        }
        default: {
            if((*token)->type != TOKEN_EOF) {
                show_error_unexpected(*token);
                next(token);
                return NULL;
            }
        }
    }
    return NULL;
}

expr_type_t ast_type(token_t **token) {
    if (*token == NULL) return UNKNOWN_TYPE;

    expr_type_t res = 0;

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

        case VOID: {
            next(token);
            res = VOID_T;
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
