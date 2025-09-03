#include "ir.h"
#include "parser.h"
#include "lexer.h"

#include <stdio.h>

// Function to convert token type to string for operators
const char* token_type_to_string(token_type_t type) {
    switch(type) {
        case PLUS: return "+";
        case MINUS: return "-";
        case STAR: return "*";
        case SLASH: return "/";
        case ASSIGN: return "=";
        default: return "?";
    }
}

// Function to convert expression type to string
const char* expr_type_to_string(expr_type_t type) {
    switch(type) {
        case INT8: return "int8";
        case INT16: return "int16";
        case INT32: return "int32";
        case INT64: return "int64";
        case UINT8: return "uint8";
        case UINT16: return "uint16";
        case UINT32: return "uint32";
        case UINT64: return "uint64";
        case F32: return "float32";
        case F64: return "float64";
        case VOID_T: return "void";
        case STRUC: return "struct";
        default: return "unknown2";
    }
}

// Function to print AST nodes (expressions)
void print_ast_node(ast_node_t *node, int depth) {
    if (node == NULL) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("(null)\n");
        return;
    }

    for (int i = 0; i < depth; i++) printf("  ");

    switch(node->type) {
        case AST_NUMBER:
            printf("NUMBER: %ld\n",node->expr.integer);
            break;

        case AST_BINARY_OP:
            printf("BINARY_OP: %s\n", token_type_to_string(node->expr.binary_op.op));

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- LEFT:\n");
            print_ast_node(node->expr.binary_op.left, depth + 2);

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- RIGHT:\n");
            print_ast_node(node->expr.binary_op.right, depth + 2);
            break;

        case AST_FUNCTION_CALL:
            printf("FUNCTION CALL: %s\n", node->expr.call.identifier ? node->expr.call.identifier : "(null)");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- ARGUMENTS (%zu):\n", node->expr.call.arg_count);
            for (size_t i = 0; i < node->expr.call.arg_count; i++) {
                print_ast_node(node->expr.call.args[i], depth + 2);
            }
            break;


        case AST_IDENTIFIER:
            printf("IDENTIFIER: %s\n", node->expr.identifier ? node->expr.identifier : "(null)");
            break;

        case AST_UNARY_OP:
            printf("UNARY_OP: %s\n", token_type_to_string(node->expr.unary_op.op));
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- OPERAND:\n");
            print_ast_node(node->expr.unary_op.node, depth + 2);
            break;

        case AST_STRING:
            printf("STRING: \"%s\"\n", node->expr.identifier ? node->expr.identifier : "(null)");
            break;

        default:
            printf("UNKNOWN_NODE_TYPE: %d\n", node->type);
            break;
    }
}

void print_ast_statement(ast_statement_t *stmt, int depth) {
    if (stmt == NULL) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("(null statement)\n");
        return;
    }

    for (int i = 0; i < depth; i++) printf("  ");

    switch(stmt->type) {
        case AST_VAR_DECLARATION:
            printf("DECLARATION:\n");

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- TYPE: %s\n", expr_type_to_string(stmt->statement.declaration.t));

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- IDENTIFIER: %s\n", stmt->statement.declaration.identifier ? stmt->statement.declaration.identifier : "(null)");

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- INITIALIZER:\n");
            if (stmt->statement.declaration.initializer != NULL) {
                print_ast_statement(stmt->statement.declaration.initializer, depth + 2);
            } else {
                for (int i = 0; i < depth + 2; i++) printf("  ");
                printf("(no initializer)\n");
            }
            break;

        case AST_VAR_ASSIGNMENT:
            printf("ASSIGNMENT:\n");

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- IDENTIFIER: %s\n", stmt->statement.assignment.identifier ? stmt->statement.assignment.identifier : "(null)");

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- VALUE:\n");
            print_ast_node(stmt->statement.assignment.value, depth + 2);
            break;

        case AST_RETURN_STMT:
            printf("RETURN:\n");

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- VALUE:\n");
            print_ast_node(stmt->statement.ret.value, depth + 2);
            break;

        case AST_FUNC_DECLARATION:
            printf("FUNCTION DECLARATION:\n");

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- RETURN TYPE: %s\n", expr_type_to_string(stmt->statement.function.type));

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- IDENTIFIER: %s\n", stmt->statement.function.identifier ? stmt->statement.function.identifier : "(null)");

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- ARGUMENTS (%zu):\n", stmt->statement.function.arg_count);
            for (size_t i = 0; i < stmt->statement.function.arg_count; i++) {
                for (int j = 0; j < depth + 2; j++) printf("  ");
                printf("%s %s\n",
                       expr_type_to_string(stmt->statement.function.args[i].type),
                       stmt->statement.function.args[i].identifier ? stmt->statement.function.args[i].identifier : "(null)");
            }

            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("+- BODY:\n");
            {
                struct block_member *bm = stmt->statement.function.block;
                int idx = 0;
                while (bm && bm->value) {
                    for (int j = 0; j < depth + 2; j++) printf("  ");
                    printf("Statement %d in block:\n", idx++);
                    print_ast_statement(bm->value, depth + 3);
                    bm = bm->next;
                }
            }
            break;

        default:
            printf("UNKNOWN_STATEMENT_TYPE: %d\n", stmt->type);
            break;
    }
}

void print_ast(struct statement_list *statements) {
    printf("=== ABSTRACT SYNTAX TREE ===\n");

    if (statements == NULL) {
        printf("(empty AST)\n");
        printf("=== END OF AST ===\n");
        return;
    }

    int stmt_count = 0;
    struct statement_list *current = statements;

    while (current != NULL && current->statement != NULL) {
        printf("\nStatement %d:\n", stmt_count++);
        print_ast_statement(current->statement, 1);
        current = current->next;
    }

    printf("\n=== END OF AST ===\n");
}

void print_tokens(token_t *token) {
    token_t *current = token;
    while(current != NULL) {
        printf("%d(%s) ", current->type, current->value);
        current = current->next;
    }
    printf("\n");
}

void print_ast_types(struct statement_list *list) {
    struct statement_list *current = list;
    printf("stmts: ");
    while (current->next != NULL) {
        printf("%d ", current->statement->type);
        current = current->next;
    }

    printf("\n");
}

void print_operand(ir_operand_t *op) {
    if (!op) {
        printf("null");
        return;
    }

    switch (op->kind) {
        case IR_OPERAND_TEMP:
            printf("t%d", op->temp_id);
            break;
        case IR_OPERAND_CONST:
            printf("%ld", op->constant.int_val);
            break;
        case IR_OPERAND_VAR:
            printf("%s", op->var_name);
            break;
        case IR_OPERAND_LABEL:
            printf("%s", op->label_name);
            break;
        case IR_OPERAND_FUNC:
            printf("%s", op->func_name);
            break;
    }
    printf("(%s)", expr_type_to_string(op->type));
}

void print_ir_instruction(ir_instruction_t *inst) {
    printf("[%s] ", expr_type_to_string(inst->result_type));
    switch (inst->opcode) {
        case IR_ADD:
            print_operand(inst->dst);
            printf(" = ");
            print_operand(inst->src1);
            printf(" + ");
            print_operand(inst->src2);
            break;
        case IR_MINUS:
            print_operand(inst->dst);
            printf(" = ");
            print_operand(inst->src1);
            printf(" - ");
            print_operand(inst->src2);
            break;
        case IR_MULT:
            print_operand(inst->dst);
            printf(" = ");
            print_operand(inst->src1);
            printf(" * ");
            print_operand(inst->src2);
            break;
        case IR_ALLOC:
            printf("alloc ");
            print_operand(inst->dst);
            break;
        case IR_STORE:
            print_operand(inst->dst);
            printf(" = ");
            print_operand(inst->src1);
            break;
        case IR_FUNC_START:
            printf("function %s(", inst->func.func_name);
            for (size_t i = 0; i < inst->func.param_count; i++) {
                if (i > 0) printf(", ");
                print_operand(inst->func.params[i]);
            }
            printf(") #%ld", inst->func.stack_size);
            break;
        case IR_FUNC_END:
            printf("end_function");
            break;
        case IR_RETURN:
            printf("return ");
            print_operand(inst->src1);
            break;
        case IR_CALL:
            print_operand(inst->dst);
            printf(" = call %s(", inst->src1->func_name);
            for (size_t i = 0; i < inst->call.arg_count; i++) {
                if (i > 0) printf(", ");
                print_operand(inst->call.args[i]);
            }
            printf(")");
            break;

        default:
            printf("unknown_op");
            break;
    }
    printf("\n");
}

void print_ir(ir_instruction_list_t *list) {
    printf("=== IR Code ===\n");
    ir_instruction_list_t *current = list;
    size_t i = 0;
    while(current != NULL && current->instruction != NULL) {
        printf("%3zu: ", i++);
        print_ir_instruction(current->instruction);
        current = current->next;
    };
    printf("===============\n");
}
