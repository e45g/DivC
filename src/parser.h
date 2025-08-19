#ifndef _PARSER_H
#define _PARSER_H

#include <stdint.h>
#include "lexer.h"

enum ast_node_type {
    AST_UNARY_OP,
    AST_BINARY_OP,
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_STRING,
    AST_ASSIGNMENT,
    AST_DECLARATION,
    AST_FUNCTION_CALL,
};

typedef enum expr_type {
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    F32,
    F64,
    VOID_T,
    STRUC,

    UNKNOWN_TYPE,

    MAX = UINT64_MAX,
} expr_type_t;

typedef struct ast_node {
    enum ast_node_type type;
    union ast_expr {
        uint64_t integer;
        char *identifier;

        struct {
            token_type_t op;
            struct ast_node *node;
        } unary_op;

        struct {
            token_type_t op;
            struct ast_node *left;
            struct ast_node *right;
        } binary_op;
    } expr;
} ast_node_t;

typedef struct ast_statement {
    enum ast_node_type type;
    union {
        struct {
            char *identifier;
            expr_type_t t;
            struct ast_statement *initializer;
        } declaration;

        struct {
            char *identifier;
            ast_node_t *value;
        } assignment;
    } statement;
} ast_statement_t;

struct statement_list {
    ast_statement_t *statement;
    struct statement_list *next;
};


struct statement_list *ast_parse(token_t *list);
ast_statement_t *ast_statement(token_t **token);
expr_type_t ast_type(token_t **token);

#endif
