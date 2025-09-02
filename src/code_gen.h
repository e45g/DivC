#ifndef _CODE_GEN_H
#define _CODE_GEN_H

#include <stdio.h>
#include "parser.h"

#define WORD_SIZE 8
#define STACK_ALIGNMENT 16

typedef enum {
    REG_RAX, // return value, arithmetic
    REG_RBX, // preserved
    REG_RCX, // arg 4
    REG_RDX, // arg 3
    REG_RSI, // arg 2
    REG_RDI, // arg 1
    REG_RSP, // stack pointer
    REG_RBP, // base pointer (stack frame)
    REG_R8,  // arg 5
    REG_R9,  // arg 6
    REG_10,  // temp
    REG_11,  // temp
} x64_registers_t;

typedef struct var_location {
    int offset; // RBP
    expr_type_t type;
    int size;
    char *identifier;
    struct var_location *next;
} var_location_t;

typedef struct tmp_location {
    int offset; // RBP
    int size;
    int temp_id;
    struct tmp_location *next;
} tmp_location_t;

typedef struct codegen_context {
    tmp_location_t *temps;
    var_location_t *locals;
    var_location_t *params;
    int stack_offset;
    int max_offset;
    char *current_function;
    FILE *output;
} codegen_context_t;

#endif
