#include <stdio.h>
#include <stdlib.h>

#include "code_gen.h"
#include "ir.h"
#include "parser.h"

const char *get_register(x64_registers_t reg, int size) {
    static const char *reg_64[] = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "rsp", "rbp", "r8",  "r9",  "r10", "r11"};

    static const char *reg_32[] = {"eax", "ebx", "ecx", "edx", "esi", "edi",
        "esp", "ebp", "r8d",  "r9d",  "r10d", "r11d"};

    static const char *reg_16[] = {"ax", "bx", "cx", "dx", "si", "di",
        "sp", "bp", "r8w",  "r9w",  "r10w", "r11w"};

    static const char *reg_8[] = {"al", "bl", "cl", "dl", "sil", "dil",
        "spl", "bpl", "r8b",  "r9b",  "r10b", "r11b"};

    switch(size) {
        case 1: return reg_8[reg];
        case 2: return reg_16[reg];
        case 4: return reg_32[reg];
        case 8: return reg_64[reg];
        default: return reg_64[reg];
    }
}

const char *get_suffix_size(int size) {
    switch(size) {
        case 1: return "b";
        case 2: return "w";
        case 4: return "l"; // long (dword)
        case 8: return "q";
        default: return "l";
    }
}

int align_offset(int offset, int alignment) {
    if(offset % alignment != 0) {
        offset -= (alignment - (abs(offset) % abs(alignment)));
    }
    return offset;
}

void generate_instruction(ir_instruction_t *instruction, codegen_context_t *ctx) {
    switch (instruction->opcode) {
        case IR_FUNC_START: {
            ctx->current_function = instruction->func.func_name;
            ctx->stack_offset = 0;
            ctx->max_offset = 0;

            fprintf(ctx->output, "\n.globl %s\n", instruction->func.func_name);
            fprintf(ctx->output, "%s:\n", instruction->func.func_name);

            if(instruction->func.param_count > 6 || instruction->func.stack_size > 0) {
                fprintf(ctx->output, "    pushq %%rbp\n");
                fprintf(ctx->output, "    movq %%rsp, %%rbp\n");
            }

            // for(int i = 0; i < instruction->func.param_count; i++) {
            //     var_location_t *param = malloc(sizeof(var_location_t));
            //     param->identifier = instruction->func.params[i]->var_name;
            //     param->next = NULL;
            // }

            break;
        }

        default:
            fprintf(ctx->output, "    # Unknown opcode: %d\n", instruction->opcode);
            break;
    }
}

void generate_x64_code(ir_instruction_list_t *inst, FILE *f) {
    codegen_context_t ctx = {0};
    ctx.output = f;

    ir_instruction_list_t *current = inst;
    while(current != NULL) {
        if(current->instruction != NULL) {
            generate_instruction(current->instruction, &ctx);
        }
        current = current->next;
    }

    fprintf(f, "\n");
}
