#include <string.h>
#include <stdlib.h>

#include "ir.h"
#include"parser.h"

ir_operand_t *create_var_operand(char *name) {
    ir_operand_t *op = malloc(sizeof(ir_operand_t));
    op->type = IR_OPERAND_VAR;
    op->var_name = strdup(name);
    return op;
}

void emit_instruction(ir_context_t *ctx, ir_instruction_t *inst) {
    ctx->instructions->instruction = inst;
    ctx->instructions->next = calloc(1, sizeof(ir_instruction_list_t));
}

void generate_statement_ir(ast_statement_t *stmt, ir_context_t *ctx) {
    if(!stmt) return;

    switch (stmt->type) {
        case AST_VAR_DECLARATION: {
            ir_instruction_t *inst = calloc(1, sizeof(ir_instruction_t));
            inst->opcode = IR_ALLOC;
            inst->dst = create_var_operand(stmt->statement.declaration.identifier);
            emit_instruction(ctx, inst);

            if(stmt->statement.declaration.initializer) {
                generate_statement_ir(stmt->statement.declaration.initializer, ctx);
            }

            break;
        }

        default: {
            break;
        }
    }
}

ir_instruction_list_t *generate_ir(struct statement_list *ast) {
    ir_context_t ctx = {0};
    ctx.label_counter = 0;
    ctx.temp_counter = 0;
    ctx.instructions = malloc(sizeof(struct ir_instruction_list));
    ir_instruction_list_t *head = ctx.instructions;

    struct statement_list *current = ast;
    while (current && current->statement) {
        generate_statement_ir(current->statement, &ctx);
        current = current->next;
    }

    return head;
}
