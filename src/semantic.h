#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "hashmap.h"
#include "parser.h"

enum symbol_kind {
    SYMBOL_FUNC,
    SYMBOL_VAR
};

struct scope {
    struct node *table[HASHMAP_TABLE_SIZE];
    struct scope *parent;
    int level;
};

struct symbol {
    enum symbol_kind kind;
    char *identifier;
    expr_type_t type;

    int scope_level;
};

struct node {
    char *key;
    symbol_t *value;
    struct node *next;
};

typedef struct symbol_table {
    struct scope *current_scope;
    struct scope *global_scope;
} symbol_table_t;

void semantic_check(struct statement_list *ast);

#endif
