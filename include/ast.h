#ifndef AST_H
#define AST_H

#include <stdlib.h>

typedef enum {
    AST_INT,
    AST_FLOAT,
    AST_STRING,
    AST_BOOL,
    AST_NULL,
    AST_VAR,
    AST_BINARY_OP,
    AST_LIST,
    AST_VAR_DECL,
    AST_CALL_STMT,
    AST_PRINT_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOREACH_STMT,
    AST_BREAK_STMT,
    AST_CONTINUE_STMT,
    AST_LOG_STMT,
    AST_MODE_DIRECTIVE,
    AST_PROGRAM
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        struct { long int_value; } int_lit;
        struct { double float_value; } float_lit;
        struct { char *string_value; } string_lit;
        struct { int bool_value; } bool_lit;
        struct { char *name; } var;
        struct {
            struct ASTNode *left;
            char op;
            struct ASTNode *right;
        } binary;
        struct {
            struct ASTNode **elements;
            int element_count;
        } list;
        struct {
            char *name;
            char *type_name;
            struct ASTNode *init_expr;
            char *var_type;
            char *watch_callback;
        } var_decl;
        struct {
            char *func_name;
            struct ASTNode **args;
            int arg_count;
        } call;
        struct {
            struct ASTNode *expr;
        } print_stmt;
        struct {
            struct ASTNode *condition;
            struct ASTNode **then_body;
            int then_count;
            struct ASTNode **else_ifs_conds;
            struct ASTNode ***else_ifs_bodies;
            int *else_ifs_counts;
            int else_if_count;
            struct ASTNode **else_body;
            int else_count;
        } if_stmt;
        struct {
            struct ASTNode *condition;
            struct ASTNode **body;
            int body_count;
        } while_stmt;
        struct {
            char *var_name;
            struct ASTNode *iterable;
            struct ASTNode **body;
            int body_count;
        } foreach_stmt;
        struct {
            struct ASTNode *message;
            char *level;
        } log_stmt;
        struct {
            char *mode;
        } mode_directive;
        struct {
            struct ASTNode **statements;
            int stmt_count;
        } program;
    };
} ASTNode;

void ast_free(ASTNode *node);

#endif