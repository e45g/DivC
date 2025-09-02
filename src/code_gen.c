#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "code_gen.h"
#include "ir.h"
#include "parser.h"

#define debug 1

// Using NASM

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

static inline int align_up(int value, int alignment) {
    return (value + alignment - 1) & -alignment;
}

static inline int align_down(int value, int alignment) {
    return value & -alignment;
}

static inline int natural_align(int size) {
    if (size >= 8) return 8;
    if (size >= 4) return 4;
    if (size >= 2) return 2;
    return 1;
}

const char *get_size_spec(int size) {
    switch(size) {
        case 1: return "byte";
        case 2: return "word";
        case 4: return "dword";
        case 8: return "qword";
        default: return "dword";
    }
}

var_location_t *find_local(codegen_context_t *ctx, char *id) {
    var_location_t *var = ctx->locals;
    while (var) {
        if (strcmp(var->identifier, id) == 0) {
            return var;
        }
        var = var->next;
    }
    return NULL;
}

tmp_location_t *find_tmp(codegen_context_t *ctx, int id) {
    tmp_location_t *temp = ctx->temps;
    while (temp) {
        if (temp->temp_id == id) {
            return temp;
        }
        temp = temp->next;
    }

    return NULL;
}

int find_tmp_offset(codegen_context_t *ctx, int id) {
    tmp_location_t *temp = ctx->temps;
    while (temp) {
        if (temp->temp_id == id) {
            return temp->offset;
        }
        temp = temp->next;
    }

    return 0;
}

var_location_t *alloc_local(codegen_context_t *ctx, char *id, int size, expr_type_t type) {
    var_location_t *var = malloc(sizeof(var_location_t));
    var->size = size;
    var->identifier = strdup(id);

    int alignment = natural_align(size);
    ctx->stack_offset = align_down(ctx->stack_offset - size, alignment);
    var->offset = ctx->stack_offset;

    var->type = type;
    var->next = ctx->locals;
    ctx->locals = var;

    return var;
}

int alloc_temp(codegen_context_t *ctx, int id, int size) {
    tmp_location_t *temp = ctx->temps;
    while (temp) {
        if (temp->temp_id == id) {
            return temp->offset;
        }
        temp = temp->next;
    }

    tmp_location_t *tmp = malloc(sizeof(tmp_location_t));
    tmp->size = size;

    int alignment = natural_align(size);
    ctx->stack_offset = align_down(ctx->stack_offset - size, alignment);
    tmp->offset = ctx->stack_offset;
    tmp->temp_id = id;
    tmp->next = ctx->temps;
    ctx->temps = tmp;

    return tmp->offset;
}

void generate_operand_load(codegen_context_t *ctx, ir_operand_t *op, x64_registers_t reg, int dst_size) {
    switch(op->kind) {
        case IR_OPERAND_CONST: {
            fprintf(ctx->output, "    mov %s, %ld\n",
                    get_register(reg, dst_size),
                    op->constant.int_val); // TODO : support other types :)
            break;
        }

        case IR_OPERAND_VAR: {
            var_location_t *var = find_local(ctx, op->var_name);
            if (var) {
                fprintf(ctx->output, "    mov %s, %s [rbp%d]\n",
                        get_register(reg, dst_size),
                        get_size_spec(dst_size),
                        var->offset);
            } else {
                fprintf(ctx->output, "    # Error: Operand %s not found\n",
                        op->var_name);
            }
            break;
        }

        case IR_OPERAND_TEMP: {
            int offset = find_tmp_offset(ctx, op->temp_id);
            if (offset == 0) {
                offset = alloc_temp(ctx, op->temp_id, dst_size);
            }
            fprintf(ctx->output, "    mov %s, %s [rbp%d]\n",
                    get_register(reg, dst_size),
                    get_size_spec(dst_size),
                    offset
                    );
            break;
        }

        default: {
            fprintf(ctx->output, "    # Unsupported operand kind\n");
            break;
        }
    }
}

void generate_operand_store(codegen_context_t *ctx, ir_operand_t *op) {
    int size = get_type_size(op->type);
    switch(op->kind) {
        case IR_OPERAND_VAR: {
            var_location_t *var = find_local(ctx, op->var_name);
            if(!var) {
                fprintf(ctx->output, "    # Error: Operand %s not found\n",
                        op->var_name);
            } else {
                fprintf(ctx->output, "    mov %s [rbp%d], %s\n",
                        get_size_spec(size),
                        var->offset,
                        get_register(REG_RAX, size)
                        );
            }
            break;
        }

        case IR_OPERAND_TEMP: {
            int offset = find_tmp_offset(ctx, op->temp_id);
            if (offset == 0) {
                offset = alloc_temp(ctx, op->temp_id, size);
            }
            fprintf(ctx->output, "    mov %s [rbp%d], %s\n",
                    get_size_spec(size),
                    offset,
                    get_register(REG_RAX, size)
                    );
            break;
        }

        default: {
            fprintf(ctx->output, "    # Unsupported operand kind\n");
            break;
        }
    }

}



void generate_instruction(ir_instruction_t *instruction, codegen_context_t *ctx) {
    switch (instruction->opcode) {
        case IR_ADD: {
            if(debug) fprintf(ctx->output, "\n    ; IR_ADD\n");
            int size = get_type_size(instruction->dst->type);
            generate_operand_load(ctx, instruction->src1, REG_RAX, size);
            generate_operand_load(ctx, instruction->src2, REG_RCX, size);
            fprintf(ctx->output, "    add %s, %s\n", get_register(REG_RAX, size), get_register(REG_RCX, size));

            generate_operand_store(ctx, instruction->dst);
            break;
        }

        case IR_MULT: {
            if(debug) fprintf(ctx->output, "\n    ; IR_MULT\n");
            int size = get_type_size(instruction->dst->type);
            generate_operand_load(ctx, instruction->src1, REG_RAX, size);
            generate_operand_load(ctx, instruction->src2, REG_RCX, size);
            // TODO: This is signed mul, support unsigned etc.
            fprintf(ctx->output, "    imul %s, %s\n", get_register(REG_RAX, size), get_register(REG_RCX, size));

            generate_operand_store(ctx, instruction->dst);
            break;
        }

        case IR_MINUS: {
            if(debug) fprintf(ctx->output, "\n    ; IR_MINUS\n");
            int size = get_type_size(instruction->dst->type);
            generate_operand_load(ctx, instruction->src1, REG_RAX, size);
            generate_operand_load(ctx, instruction->src2, REG_RCX, size);
            fprintf(ctx->output, "    sub %s, %s\n", get_register(REG_RAX, size), get_register(REG_RCX, size));

            generate_operand_store(ctx, instruction->dst);
            break;
        }

        case IR_ALLOC: {
            if(debug) fprintf(ctx->output, "\n    ; IR_ALLOC\n");
            expr_type_t type = instruction->dst->type;
            int size = get_type_size(type);
            alloc_local(ctx, instruction->dst->var_name, size, type);
            break;
        }

        case IR_STORE: {
            if(debug) fprintf(ctx->output, "\n    ; IR_STORE\n");
            generate_operand_load(ctx, instruction->src1, REG_RAX, get_type_size(instruction->dst->type));
            generate_operand_store(ctx, instruction->dst);
            break;
        }

        case IR_FUNC_START: {
            if(debug) fprintf(ctx->output, "\n; FUNC_START\n");
            ctx->current_function = instruction->func.func_name;
            ctx->stack_offset = 0;
            ctx->max_offset = align_up(instruction->func.stack_size, 16); // ik rsp has to be aligned to 16 by callee but this? idk

            fprintf(ctx->output, "\nglobal %s\n", instruction->func.func_name);
            fprintf(ctx->output, "%s:\n", instruction->func.func_name);

            // TODO: when needed
            fprintf(ctx->output, "    push rbp\n");
            fprintf(ctx->output, "    mov rbp, rsp\n");

            // TODO: when needed
            // if(instruction->func.stack_size > 0) {
            // TODO: make this correctly >:(
            size_t tmp = 0;
            for(size_t i = 0; i < instruction->func.param_count && i < 6; i++) {
                tmp += get_type_size(instruction->func.params[i]->type);
            }
            fprintf(ctx->output, "    sub rsp, %d\n", align_up(instruction->func.stack_size+tmp, 16));
            // }

            // TODO: Only when needed :)
            for(size_t i = 0; i < instruction->func.param_count && i < 6; i++) {
                int size = get_type_size(instruction->func.params[i]->type);
                var_location_t *var = alloc_local(ctx,
                                                  instruction->func.params[i]->var_name,
                                                  size,
                                                  instruction->func.params[i]->type);

                x64_registers_t call_regs[] = {REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9};

                fprintf(ctx->output, "    mov %s [rbp%d], %s\n",
                        get_size_spec(size),
                        var->offset,
                        get_register(call_regs[i], size));
            }

            break;
        }

        case IR_FUNC_END: {
            if(debug) fprintf(ctx->output, "\n; FUNC_END\n");
            fprintf(ctx->output, ".%s_end:\n", ctx->current_function);
            fprintf(ctx->output, "    leave\n"); // TODO: only when needed
            if(strcmp(ctx->current_function, "_start") == 0) {
                fprintf(ctx->output, "    mov rdi, rax\n");
                fprintf(ctx->output, "    mov rax, 60\n");
                fprintf(ctx->output, "    syscall\n");
            }
            else {
                fprintf(ctx->output, "    ret\n");
            }
            break;
        }

        // TODO: get function return size, not the src size
        case IR_RETURN: {
            if(debug) fprintf(ctx->output, "\n    ; IR_RETURN\n");
            generate_operand_load(ctx, instruction->src1, REG_RAX, get_type_size(INT32)); // TODO : get function type
            fprintf(ctx->output, "    jmp .%s_end\n", ctx->current_function);
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
