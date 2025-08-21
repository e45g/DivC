#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "parser.h"
typedef enum {
    SYM_VAR,
    SYM_FUNC,
    SYM_TYPE, // user-defined type (structs)
} symbol_kind_t;

typedef struct {
    char *identifier;
    symbol_kind_t kind;
    expr_type_t type;
    // int scope_level, line, column
} symbol_t;

void semantic_check(struct statement_list *ast);

#endif
