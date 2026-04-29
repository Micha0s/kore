#include "ast.h"
#include <stdlib.h>

void ast_free(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_STRING:
            free(node->string_lit.string_value);
            break;
        case AST_VAR:
            free(node->var.name);
            break;
        case AST_VAR_DECL:
            free(node->var_decl.name);
            free(node->var_decl.type_name);
            free(node->var_decl.init_expr);
            free(node->var_decl.var_type);
            if (node->var_decl.watch_callback) free(node->var_decl.watch_callback);
            break;
        case AST_CALL_STMT:
            free(node->call.func_name);
            for (int i = 0; i < node->call.arg_count; i++) {
                ast_free(node->call.args[i]);
            }
            free(node->call.args);
            break;
        case AST_PRINT_STMT:
            ast_free(node->print_stmt.expr);
            break;
        case AST_IF_STMT:
            ast_free(node->if_stmt.condition);
            for (int i = 0; i < node->if_stmt.then_count; i++) ast_free(node->if_stmt.then_body[i]);
            free(node->if_stmt.then_body);
            for (int i = 0; i < node->if_stmt.else_if_count; i++) {
                ast_free(node->if_stmt.else_ifs_conds[i]);
                for (int j = 0; j < node->if_stmt.else_ifs_counts[i]; j++) {
                    ast_free(node->if_stmt.else_ifs_bodies[i][j]);
                }
                free(node->if_stmt.else_ifs_bodies[i]);
            }
            free(node->if_stmt.else_ifs_conds);
            free(node->if_stmt.else_ifs_bodies);
            free(node->if_stmt.else_ifs_counts);
            if (node->if_stmt.else_body) {
                for (int i = 0; i < node->if_stmt.else_count; i++) ast_free(node->if_stmt.else_body[i]);
                free(node->if_stmt.else_body);
            }
            break;
        case AST_WHILE_STMT:
            ast_free(node->while_stmt.condition);
            for (int i = 0; i < node->while_stmt.body_count; i++) ast_free(node->while_stmt.body[i]);
            free(node->while_stmt.body);
            break;
        case AST_FOREACH_STMT:
            free(node->foreach_stmt.var_name);
            ast_free(node->foreach_stmt.iterable);
            for (int i = 0; i < node->foreach_stmt.body_count; i++) ast_free(node->foreach_stmt.body[i]);
            free(node->foreach_stmt.body);
            break;
        case AST_LOG_STMT:
            ast_free(node->log_stmt.message);
            free(node->log_stmt.level);
            break;
        case AST_MODE_DIRECTIVE:
            free(node->mode_directive.mode);
            break;
        case AST_LIST:
            for (int i = 0; i < node->list.element_count; i++) ast_free(node->list.elements[i]);
            free(node->list.elements);
            break;
        case AST_BINARY_OP:
            ast_free(node->binary.left);
            ast_free(node->binary.right);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < node->program.stmt_count; i++) {
                ast_free(node->program.statements[i]);
            }
            free(node->program.statements);
            break;
        default: break;
    }
    free(node);
}