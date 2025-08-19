#ifndef _PARSER_H
#define _PARSER_H

#include <stddef.h>
#include <stdint.h>
#include "lexer.h"

#define TYPE_POINTER 0x100

enum ast_expr_type {
    AST_UNARY_OP,
    AST_BINARY_OP,
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_STRING,
    AST_FUNCTION_CALL,
};

enum ast_stmt_type {
    AST_VAR_DECLARATION,
    AST_VAR_ASSIGNMENT,
    AST_FUNC_DECLARATION,
    AST_RETURN_STMT,
    AST_EXPRESSION_STMT,
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

struct arg {
    expr_type_t type;
    char *identifier;
};

struct block_member;

typedef struct ast_node {
    enum ast_expr_type type;
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
    enum ast_stmt_type type;
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

        struct {
            ast_node_t *value;
        } ret;

        struct {
            expr_type_t type;
            char *identifier;
            struct arg *args;
            size_t arg_count;
            struct block_member *block;
        } function;

    } statement;
} ast_statement_t;

struct statement_list {
    ast_statement_t *statement;
    struct statement_list *next;
};

struct block_member {
    ast_statement_t *value;
    size_t stack_size;
    struct block_member *next;
};


struct statement_list *ast_parse(token_t *list);
ast_statement_t *ast_statement(token_t **token);
expr_type_t ast_type(token_t **token);
ast_node_t *expression(token_t **token);

#endif
