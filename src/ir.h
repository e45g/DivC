#ifndef _IR_H
#define _IR_H

#include "parser.h"
#include <stdint.h>

enum ir_opcode {
    IR_MULT,
    IR_ADD,
    IR_MINUS,

    IR_ALLOC,
    IR_STORE,

    IR_CALL,

    IR_FUNC_START,
    IR_FUNC_END,
    IR_RETURN,
};

enum ir_operand_kind {
    IR_OPERAND_TEMP,
    IR_OPERAND_CONST,
    IR_OPERAND_VAR,
    IR_OPERAND_LABEL,
    IR_OPERAND_FUNC
};

typedef struct {
    enum ir_operand_kind kind;
    expr_type_t type;
    union {
        int temp_id; // e.g. t1, t2
        char *var_name;

        struct {
            union {
                int64_t int_val;
                double float_val;
                char *string_val;
            };
        } constant;

        char *label_name;
        char *func_name; // for calls
    };
} ir_operand_t;

typedef struct {
    enum ir_opcode opcode;
    expr_type_t result_type;
    ir_operand_t *dst;
    ir_operand_t *src1;
    ir_operand_t *src2;

    union {
        struct {
            ir_operand_t **args;
            size_t arg_count;
        } call;

        struct {
            char *func_name;
            ir_operand_t **params;
            size_t param_count;
            size_t stack_size;
            expr_type_t return_type;
        } func;
    };
} ir_instruction_t;

typedef struct ir_instruction_list {
    ir_instruction_t *instruction;
    struct ir_instruction_list *next;
} ir_instruction_list_t;

typedef struct ir_context {
    struct ir_instruction_list *instructions;
    int temp_counter;
    int label_counter;
} ir_context_t;

ir_instruction_list_t *generate_ir(struct statement_list *ast);

#endif
