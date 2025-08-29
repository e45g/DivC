#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ir.h"
#include "lexer.h"
#include "parser.h"

ir_operand_t *create_const_operand(int64_t value) {
    ir_operand_t *op = malloc(sizeof(ir_operand_t));
    op->type = IR_OPERAND_CONST;
    op->constant.int_val = value;
    op->constant.type = INT32; // default to 32 cuz why not
    return op;
}

ir_operand_t *create_var_operand(char *name) {
    ir_operand_t *op = malloc(sizeof(ir_operand_t));
    op->type = IR_OPERAND_VAR;
    op->var_name = strdup(name);
    return op;
}

ir_operand_t *create_tmp_operand(int id) {
    ir_operand_t *op = malloc(sizeof(ir_operand_t));
    op->type = IR_OPERAND_TEMP;
    op->temp_id = id;
    return op;
}

int new_temp(ir_context_t *ctx) {
    return ctx->temp_counter++;
}

char *new_label(ir_context_t *ctx) {
    char *label = malloc(32); // TODO: this is obv enough, but magic numbers aren't good
    snprintf(label, 32, "L%d", ctx->label_counter++);
    return label;
}

void emit_instruction(ir_context_t *ctx, ir_instruction_t *inst) {
    ctx->instructions->instruction = inst;
    ctx->instructions->next = calloc(1, sizeof(ir_instruction_list_t));
    ctx->instructions = ctx->instructions->next;
}

ir_operand_t *generate_expr_ir(ast_node_t *expr, ir_context_t *ctx) {
    if(!expr) return NULL;

    switch(expr->type) {
        case AST_NUMBER: {
            return create_const_operand(expr->expr.integer);
        }

        case AST_IDENTIFIER: {
            return create_var_operand(expr->expr.identifier);
        }

        case AST_BINARY_OP: {
            ir_operand_t *left = generate_expr_ir(expr->expr.binary_op.left, ctx);
            ir_operand_t *right = generate_expr_ir(expr->expr.binary_op.right, ctx);

            int temp_id = new_temp(ctx);
            ir_operand_t *res = create_tmp_operand(temp_id);

            ir_instruction_t *inst = calloc(1, sizeof(ir_instruction_t));
            inst->dst = res;
            inst->src1 = left;
            inst->src2 = right;

            switch(expr->expr.binary_op.op) {
                case PLUS:
                    inst->opcode = IR_ADD;
                    break;

                case MINUS:
                    inst->opcode = IR_MINUS;
                    break;

                case STAR:
                    inst->opcode = IR_MULT;
                    break;

                default:
                    inst->opcode = IR_ADD; // fallback, why +? who doesnt like plus?
                    break;
            }
            emit_instruction(ctx, inst);
            return res;
        }

        default: {
            return NULL;
        }
    }
}

void generate_statement_ir(ast_statement_t *stmt, ir_context_t *ctx);
void generate_block_ir(struct block_member *blck_meber, ir_context_t *ctx) {
    while(blck_meber != NULL) {
        if(blck_meber->value != NULL) {
            generate_statement_ir(blck_meber->value, ctx);
        }
        blck_meber = blck_meber->next;
    }
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

        case AST_VAR_ASSIGNMENT: {
            ir_operand_t *value = generate_expr_ir(stmt->statement.assignment.value, ctx);
            ir_instruction_t *inst = calloc(1, sizeof(ir_instruction_t));
            inst->opcode = IR_STORE;
            inst->dst = create_var_operand(stmt->statement.assignment.identifier);
            inst->src1 = value;
            emit_instruction(ctx, inst);

            break;

        }

        case AST_RETURN_STMT: {
            ir_operand_t *val = generate_expr_ir(stmt->statement.ret.value, ctx);

            ir_instruction_t *inst = calloc(1, sizeof(ir_instruction_t));
            inst->opcode = IR_RETURN;
            inst->src1 = val;

            emit_instruction(ctx, inst);
            break;
        }

        case AST_FUNC_DECLARATION: {
            ir_instruction_t *inst_start = calloc(1, sizeof(ir_instruction_t));
            inst_start->opcode = IR_FUNC_START;
            inst_start->func.func_name = strdup(stmt->statement.function.identifier);
            inst_start->func.param_count = stmt->statement.function.arg_count;
            inst_start->func.return_type = stmt->statement.function.type;

            if (stmt->statement.function.arg_count > 0) {
                inst_start->func.params = malloc(sizeof(ir_operand_t*) * stmt->statement.function.arg_count);
                for (size_t i = 0; i < stmt->statement.function.arg_count; i++) {
                    inst_start->func.params[i] = create_var_operand(stmt->statement.function.args[i].identifier);
                }
            }

            emit_instruction(ctx, inst_start);
            generate_block_ir(stmt->statement.function.block, ctx);

            ir_instruction_t *inst_end = calloc(1, sizeof(ir_instruction_t));
            inst_end->opcode = IR_FUNC_END;
            emit_instruction(ctx, inst_end);

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
