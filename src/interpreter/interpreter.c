#include "interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARS 100
typedef struct {
    char *name;
    union {
        long int_val;
        double float_val;
        char *str_val;
        int bool_val;
        struct {
            ASTNode **elements;
            int count;
        } list_val;
    };
    char *type;
    char *callback_name;
} Var;

static Var g_vars[MAX_VARS];
static int g_var_count = 0;
static char *g_mode = "silent";
static int g_break_flag = 0;
static int g_continue_flag = 0;

static Var *get_var(const char *name) {
    for (int i = 0; i < g_var_count; i++) {
        if (strcmp(g_vars[i].name, name) == 0) return &g_vars[i];
    }
    return NULL;
}

static ASTNode *eval(ASTNode *node);

static void set_var(const char *name, ASTNode *value_node) {
    Var *v = get_var(name);
    if (!v) {
        v = &g_vars[g_var_count++];
        v->name = strdup(name);
        v->type = NULL;
        v->callback_name = NULL;
    }
    ASTNode *val = eval(value_node);
    switch (val->type) {
        case AST_INT:
            v->type = "int";
            v->int_val = val->int_lit.int_value;
            break;
        case AST_FLOAT:
            v->type = "float";
            v->float_val = val->float_lit.float_value;
            break;
        case AST_STRING:
            v->type = "string";
            v->str_val = strdup(val->string_lit.string_value);
            break;
        case AST_BOOL:
            v->type = "bool";
            v->bool_val = val->bool_lit.bool_value;
            break;
        case AST_LIST:
            v->type = "list";
            v->list_val.elements = val->list.elements;
            v->list_val.count = val->list.element_count;
            break;
        default: break;
    }

    if (v->callback_name) {
        printf("[watch] %s changed to ", v->name);
        if (v->type) {
            if (strcmp(v->type, "int") == 0) printf("%ld", v->int_val);
            else if (strcmp(v->type, "float") == 0) printf("%f", v->float_val);
            else if (strcmp(v->type, "string") == 0) printf("'%s'", v->str_val);
            else if (strcmp(v->type, "bool") == 0) printf("%s", v->bool_val ? "true" : "false");
        }
        printf("\n");
    }
}

static ASTNode *eval_var(const char *name) {
    Var *v = get_var(name);
    if (!v) { fprintf(stderr, "Variable '%s' not defined\n", name); exit(1); }
    ASTNode *node = malloc(sizeof(ASTNode));
    if (strcmp(v->type, "int") == 0) {
        node->type = AST_INT;
        node->int_lit.int_value = v->int_val;
    } else if (strcmp(v->type, "float") == 0) {
        node->type = AST_FLOAT;
        node->float_lit.float_value = v->float_val;
    } else if (strcmp(v->type, "string") == 0) {
        node->type = AST_STRING;
        node->string_lit.string_value = strdup(v->str_val);
    } else if (strcmp(v->type, "bool") == 0) {
        node->type = AST_BOOL;
        node->bool_lit.bool_value = v->bool_val;
    } else if (strcmp(v->type, "list") == 0) {
        node->type = AST_LIST;
        node->list.elements = v->list_val.elements;
        node->list.element_count = v->list_val.count;
    }
    return node;
}

static ASTNode *eval(ASTNode *node) {
    if (!node) return NULL;
    switch (node->type) {
        case AST_INT:
        case AST_FLOAT:
        case AST_STRING:
        case AST_BOOL:
        case AST_NULL:
            return node;
        case AST_VAR:
            return eval_var(node->var.name);
        case AST_LIST:
            return node;
        case AST_BINARY_OP: {
            ASTNode *left = eval(node->binary.left);
            ASTNode *right = eval(node->binary.right);
            ASTNode *result = malloc(sizeof(ASTNode));
            if (left->type == AST_INT && right->type == AST_INT) {
                result->type = AST_INT;
                long l = left->int_lit.int_value;
                long r = right->int_lit.int_value;
                switch (node->binary.op) {
                    case '+': result->int_lit.int_value = l + r; break;
                    case '-': result->int_lit.int_value = l - r; break;
                    case '*': result->int_lit.int_value = l * r; break;
                    case '/': result->int_lit.int_value = l / r; break;
                    case '=': result->type = AST_BOOL; result->bool_lit.bool_value = (l == r); break;
                    case '!': result->type = AST_BOOL; result->bool_lit.bool_value = (l != r); break;
                    case '<': result->type = AST_BOOL; result->bool_lit.bool_value = (l < r); break;
                    case 'L': result->type = AST_BOOL; result->bool_lit.bool_value = (l <= r); break;
                    case '>': result->type = AST_BOOL; result->bool_lit.bool_value = (l > r); break;
                    case 'G': result->type = AST_BOOL; result->bool_lit.bool_value = (l >= r); break;
                    default: fprintf(stderr, "Unknown op\n"); exit(1);
                }
            } else {
                double l = (left->type == AST_INT) ? left->int_lit.int_value : left->float_lit.float_value;
                double r = (right->type == AST_INT) ? right->int_lit.int_value : right->float_lit.float_value;
                result->type = (node->binary.op == '=' || node->binary.op == '!' || 
                                node->binary.op == '<' || node->binary.op == 'L' ||
                                node->binary.op == '>' || node->binary.op == 'G') ? AST_BOOL : AST_FLOAT;
                if (result->type == AST_BOOL) {
                    switch (node->binary.op) {
                        case '=': result->bool_lit.bool_value = (l == r); break;
                        case '!': result->bool_lit.bool_value = (l != r); break;
                        case '<': result->bool_lit.bool_value = (l < r); break;
                        case 'L': result->bool_lit.bool_value = (l <= r); break;
                        case '>': result->bool_lit.bool_value = (l > r); break;
                        case 'G': result->bool_lit.bool_value = (l >= r); break;
                        default: fprintf(stderr, "Unknown op\n"); exit(1);
                    }
                } else {
                    switch (node->binary.op) {
                        case '+': result->float_lit.float_value = l + r; break;
                        case '-': result->float_lit.float_value = l - r; break;
                        case '*': result->float_lit.float_value = l * r; break;
                        case '/': result->float_lit.float_value = l / r; break;
                        default: fprintf(stderr, "Unknown op\n"); exit(1);
                    }
                }
            }
            return result;
        }
        default:
            fprintf(stderr, "eval: unsupported node type %d\n", node->type);
            exit(1);
    }
}

static void execute_stmt(ASTNode *stmt) {
    switch (stmt->type) {
        case AST_VAR_DECL:
            set_var(stmt->var_decl.name, stmt->var_decl.init_expr);
            if (stmt->var_decl.watch_callback) {
                Var *v = get_var(stmt->var_decl.name);
                if (v) {
                    v->callback_name = strdup(stmt->var_decl.watch_callback);
                }
            }
            break;
        case AST_CALL_STMT:
            if (strcmp(stmt->call.func_name, "print") == 0) {
                for (int j = 0; j < stmt->call.arg_count; j++) {
                    ASTNode *arg = eval(stmt->call.args[j]);
                    if (arg->type == AST_INT) printf("%ld", arg->int_lit.int_value);
                    else if (arg->type == AST_FLOAT) printf("%f", arg->float_lit.float_value);
                    else if (arg->type == AST_STRING) printf("%s", arg->string_lit.string_value);
                    else if (arg->type == AST_BOOL) printf("%s", arg->bool_lit.bool_value ? "true" : "false");
                }
                printf("\n");
            } else {
                fprintf(stderr, "Unknown function '%s'\n", stmt->call.func_name);
            }
            break;
        case AST_PRINT_STMT: {
            ASTNode *arg = eval(stmt->print_stmt.expr);
            if (arg->type == AST_INT) printf("%ld\n", arg->int_lit.int_value);
            else if (arg->type == AST_FLOAT) printf("%f\n", arg->float_lit.float_value);
            else if (arg->type == AST_STRING) printf("%s\n", arg->string_lit.string_value);
            break;
        }
        case AST_IF_STMT: {
            ASTNode *cond = eval(stmt->if_stmt.condition);
            int truthy = 0;
            if (cond->type == AST_BOOL) truthy = cond->bool_lit.bool_value;
            else if (cond->type == AST_INT) truthy = cond->int_lit.int_value != 0;
            else if (cond->type == AST_FLOAT) truthy = cond->float_lit.float_value != 0.0;
            else if (cond->type == AST_STRING) truthy = strlen(cond->string_lit.string_value) > 0;
            if (truthy) {
                for (int i = 0; i < stmt->if_stmt.then_count; i++) {
                    execute_stmt(stmt->if_stmt.then_body[i]);
                }
            } else {
                int executed = 0;
                for (int i = 0; i < stmt->if_stmt.else_if_count; i++) {
                    cond = eval(stmt->if_stmt.else_ifs_conds[i]);
                    truthy = 0;
                    if (cond->type == AST_BOOL) truthy = cond->bool_lit.bool_value;
                    else if (cond->type == AST_INT) truthy = cond->int_lit.int_value != 0;
                    else if (cond->type == AST_FLOAT) truthy = cond->float_lit.float_value != 0.0;
                    else if (cond->type == AST_STRING) truthy = strlen(cond->string_lit.string_value) > 0;
                    if (truthy) {
                        for (int j = 0; j < stmt->if_stmt.else_ifs_counts[i]; j++) {
                            execute_stmt(stmt->if_stmt.else_ifs_bodies[i][j]);
                        }
                        executed = 1;
                        break;
                    }
                }
                if (!executed && stmt->if_stmt.else_body) {
                    for (int i = 0; i < stmt->if_stmt.else_count; i++) {
                        execute_stmt(stmt->if_stmt.else_body[i]);
                    }
                }
            }
            break;
        }
        case AST_WHILE_STMT: {
            while (1) {
                ASTNode *cond = eval(stmt->while_stmt.condition);
                int truthy = 0;
                if (cond->type == AST_BOOL) truthy = cond->bool_lit.bool_value;
                else if (cond->type == AST_INT) truthy = cond->int_lit.int_value != 0;
                else if (cond->type == AST_FLOAT) truthy = cond->float_lit.float_value != 0.0;
                else if (cond->type == AST_STRING) truthy = strlen(cond->string_lit.string_value) > 0;
                if (!truthy) break;

                for (int i = 0; i < stmt->while_stmt.body_count; i++) {
                    execute_stmt(stmt->while_stmt.body[i]);
                    if (g_break_flag || g_continue_flag) break;
                }
                if (g_break_flag) {
                    g_break_flag = 0;
                    break;
                }
                if (g_continue_flag) {
                    g_continue_flag = 0;
                    continue;
                }
            }
            break;
        }
        case AST_FOREACH_STMT: {
            ASTNode *iterable = eval(stmt->foreach_stmt.iterable);
            if (!iterable || iterable->type != AST_LIST) {
                fprintf(stderr, "foreach requires a list\n");
                exit(1);
            }
            char loop_var_name[256];
            strcpy(loop_var_name, stmt->foreach_stmt.var_name);
            
            for (int i = 0; i < iterable->list.element_count; i++) {
                ASTNode *elem = iterable->list.elements[i];
                ASTNode *val = eval(elem);
                
                Var *v = get_var(loop_var_name);
                if (!v) {
                    v = &g_vars[g_var_count++];
                    v->name = strdup(loop_var_name);
                }
                v->type = NULL;
                switch (val->type) {
                    case AST_INT:    v->type = "int";    v->int_val = val->int_lit.int_value; break;
                    case AST_FLOAT:  v->type = "float";  v->float_val = val->float_lit.float_value; break;
                    case AST_STRING: v->type = "string"; v->str_val = strdup(val->string_lit.string_value); break;
                    case AST_BOOL:   v->type = "bool";   v->bool_val = val->bool_lit.bool_value; break;
                    default: break;
                }
                for (int j = 0; j < stmt->foreach_stmt.body_count; j++) {
                    execute_stmt(stmt->foreach_stmt.body[j]);
                }
            }
            break;
        }
        case AST_BREAK_STMT:
            g_break_flag = 1;
            break;
        case AST_CONTINUE_STMT:
            g_continue_flag = 1;
            break;
        case AST_LOG_STMT: {
            if (strcmp(g_mode, "silent") != 0) {
                ASTNode *msg = eval(stmt->log_stmt.message);
                fprintf(stderr, "[%s] ", stmt->log_stmt.level);
                if (msg->type == AST_INT) fprintf(stderr, "%ld", msg->int_lit.int_value);
                else if (msg->type == AST_FLOAT) fprintf(stderr, "%f", msg->float_lit.float_value);
                else if (msg->type == AST_STRING) fprintf(stderr, "%s", msg->string_lit.string_value);
                else if (msg->type == AST_BOOL) fprintf(stderr, "%s", msg->bool_lit.bool_value ? "true" : "false");
                fprintf(stderr, "\n");
            }
            break;
        }
        case AST_MODE_DIRECTIVE:
            g_mode = stmt->mode_directive.mode;
            break;
        default: break;
    }
}

void interpret(ASTNode *program) {
    for (int i = 0; i < program->program.stmt_count; i++) {
        execute_stmt(program->program.statements[i]);
    }
}