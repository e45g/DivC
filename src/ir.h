#ifndef __IR_H
#define __IR_H

#include "parser.h"
#include <stdint.h>

enum ir_opcode {
    IR_MULT,
    IR_ADD,
    IR_MINUS,
};

enum ir_operand_kind {
    IR_OPERAND_TEMP,
    IR_OPERAND_CONST,
    IR_OPERAND_VAR,
    IR_OPERAND_LABEL,
    IR_OPERAND_FUNC
};

typedef struct {
    enum ir_operand_kind type;
    union {
        int temp_id; // e.g. t1, t2
        char *var_name;

        struct {
            expr_type_t type;
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
            expr_type_t return_type;
        } func;
    };
} ir_instruction_t;



#endif
