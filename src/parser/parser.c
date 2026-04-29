#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Lexer *g_lexer = NULL;
static Token *g_current_token = NULL;

static void advance_token() {
    if (g_current_token) {
        if (g_current_token->value && 
            (g_current_token->type == TOKEN_IDENTIFIER || 
             g_current_token->type == TOKEN_STRING ||
             g_current_token->type == TOKEN_INT ||
             g_current_token->type == TOKEN_FLOAT ||
             g_current_token->type == TOKEN_KW_VAR ||
             g_current_token->type == TOKEN_KW_CONST ||
             g_current_token->type == TOKEN_KW_FUNC ||
             g_current_token->type == TOKEN_TYPE_MAYBE ||
             g_current_token->type == TOKEN_TYPE_ONCE ||
             g_current_token->type == TOKEN_TYPE_WATCH)) {
            free(g_current_token->value);
        }
        free(g_current_token);
    }
    g_current_token = lexer_next(g_lexer);
}

static TokenType current_type() {
    return g_current_token ? g_current_token->type : TOKEN_EOF;
}

static void expect(TokenType type, const char *msg) {
    if (current_type() != type) {
        fprintf(stderr, "Parse error at %d:%d: %s (got %s)\n", 
                g_current_token->line, g_current_token->column, msg,
                token_type_names[current_type()]);
        exit(1);
    }
    advance_token();
}

static ASTNode *parse_expression(void);
static ASTNode *parse_statement(void);

static ASTNode *parse_primary() {
    ASTNode *node = malloc(sizeof(ASTNode));
    Token *tok = g_current_token;

    switch (tok->type) {
        case TOKEN_INT:
            node->type = AST_INT;
            node->int_lit.int_value = atol(tok->value);
            advance_token();
            break;
        case TOKEN_FLOAT:
            node->type = AST_FLOAT;
            node->float_lit.float_value = atof(tok->value);
            advance_token();
            break;
        case TOKEN_STRING:
            node->type = AST_STRING;
            node->string_lit.string_value = strdup(tok->value);
            advance_token();
            break;
        case TOKEN_TRUE:
            node->type = AST_BOOL;
            node->bool_lit.bool_value = 1;
            advance_token();
            break;
        case TOKEN_FALSE:
            node->type = AST_BOOL;
            node->bool_lit.bool_value = 0;
            advance_token();
            break;
        case TOKEN_NULL:
            node->type = AST_NULL;
            advance_token();
            break;
        case TOKEN_IDENTIFIER:
            node->type = AST_VAR;
            node->var.name = strdup(tok->value);
            advance_token();
            break;
        case TOKEN_LPAREN:
            advance_token();
            free(node);
            node = parse_expression();
            expect(TOKEN_RPAREN, "Expected ')'");
            break;
        case TOKEN_LBRACE:
            advance_token();
            free(node);
            node = parse_expression();
            expect(TOKEN_RBRACE, "Expected '}'");
            break;
        case TOKEN_LBRACKET: {
            advance_token();
            ASTNode *list_node = malloc(sizeof(ASTNode));
            list_node->type = AST_LIST;
            ASTNode **elems = malloc(sizeof(ASTNode*) * 100);
            int e_idx = 0;
            if (current_type() != TOKEN_RBRACKET) {
                elems[e_idx++] = parse_expression();
                while (current_type() == TOKEN_COMMA) {
                    advance_token();
                    elems[e_idx++] = parse_expression();
                }
            }
            expect(TOKEN_RBRACKET, "Expected ']'");
            list_node->list.elements = elems;
            list_node->list.element_count = e_idx;
            free(node);
            return list_node;
        }
        default:
            fprintf(stderr, "Unexpected token in expression: %s\n", token_type_names[tok->type]);
            exit(1);
    }
    return node;
}

static ASTNode *parse_multiplicative() {
    ASTNode *left = parse_primary();
    while (current_type() == TOKEN_STAR || current_type() == TOKEN_SLASH) {
        TokenType op_type = current_type();
        char op = (op_type == TOKEN_STAR) ? '*' : '/';
        advance_token();
        ASTNode *right = parse_primary();
        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINARY_OP;
        new_node->binary.left = left;
        new_node->binary.op = op;
        new_node->binary.right = right;
        left = new_node;
    }
    return left;
}

static ASTNode *parse_additive() {
    ASTNode *left = parse_multiplicative();
    while (current_type() == TOKEN_PLUS || current_type() == TOKEN_MINUS) {
        TokenType op_type = current_type();
        char op = (op_type == TOKEN_PLUS) ? '+' : '-';
        advance_token();
        ASTNode *right = parse_multiplicative();
        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINARY_OP;
        new_node->binary.left = left;
        new_node->binary.op = op;
        new_node->binary.right = right;
        left = new_node;
    }
    return left;
}

static ASTNode *parse_comparison() {
    ASTNode *left = parse_additive();
    while (1) {
        TokenType op_type = current_type();
        char op = 0;
        if (op_type == TOKEN_EQEQ) op = '=';
        else if (op_type == TOKEN_NOTEQ) op = '!';
        else if (op_type == TOKEN_LT) op = '<';
        else if (op_type == TOKEN_LTEQ) op = 'L';
        else if (op_type == TOKEN_GT) op = '>';
        else if (op_type == TOKEN_GTEQ) op = 'G';
        else break;
        advance_token();
        ASTNode *right = parse_additive();
        ASTNode *new_node = malloc(sizeof(ASTNode));
        new_node->type = AST_BINARY_OP;
        new_node->binary.left = left;
        new_node->binary.op = op;
        new_node->binary.right = right;
        left = new_node;
    }
    return left;
}

static ASTNode *parse_expression() {
    return parse_comparison();
}

static ASTNode *parse_var_decl() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VAR_DECL;

    expect(TOKEN_KW_INIT, "Expected '--init'");
    Token *name_tok = g_current_token;
    char *var_name = strdup(name_tok->value);
    expect(TOKEN_IDENTIFIER, "Expected variable name");
    node->var_decl.name = var_name;

    expect(TOKEN_LBRACKET, "Expected '['");
    expect(TOKEN_LBRACE, "Expected '{'");
    Token *type_tok = g_current_token;
    char *type_name = strdup(type_tok->value);
    expect(TOKEN_IDENTIFIER, "Expected type name");
    node->var_decl.type_name = type_name;
    expect(TOKEN_RBRACE, "Expected '}'");
    expect(TOKEN_COMMA, "Expected ','");

    node->var_decl.init_expr = parse_expression();

    expect(TOKEN_RBRACKET, "Expected ']'");
    expect(TOKEN_KW_TYPE, "Expected '--type'");

    Token *var_type_tok = g_current_token;
    char *var_type_str = NULL;
    if (current_type() == TOKEN_KW_VAR || current_type() == TOKEN_KW_CONST ||
        current_type() == TOKEN_TYPE_ONCE || current_type() == TOKEN_TYPE_WATCH ||
        current_type() == TOKEN_TYPE_MAYBE) {
        var_type_str = strdup(var_type_tok->value);
        advance_token();
    } else {
        fprintf(stderr, "Expected var type keyword\n");
        exit(1);
    }
    node->var_decl.var_type = var_type_str ? strdup(var_type_str) : strdup("var");
    node->var_decl.watch_callback = NULL;

    if (var_type_str && strcmp(var_type_str, "watch") == 0) {
        if (current_type() == TOKEN_IDENTIFIER) {
            Token *cb = g_current_token;
            node->var_decl.watch_callback = strdup(cb->value);
            advance_token();
        } else {
            fprintf(stderr, "Expected callback name after '--type watch'\n");
            exit(1);
        }
    }

    return node;
}

static ASTNode *parse_call_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_CALL_STMT;

    expect(TOKEN_KW_CALL, "Expected '--call'");
    Token *func_tok = g_current_token;
    char *fname = strdup(func_tok->value);
    expect(TOKEN_IDENTIFIER, "Expected function name");
    node->call.func_name = fname;

    expect(TOKEN_LPAREN, "Expected '('");
    ASTNode **args = malloc(sizeof(ASTNode*) * 8);
    int arg_idx = 0;
    if (current_type() != TOKEN_RPAREN) {
        args[arg_idx++] = parse_expression();
        while (current_type() == TOKEN_COMMA) {
            advance_token();
            args[arg_idx++] = parse_expression();
        }
    }
    node->call.args = args;
    node->call.arg_count = arg_idx;
    expect(TOKEN_RPAREN, "Expected ')'");
    expect(TOKEN_SEMICOLON, "Expected ';'");

    return node;
}

static ASTNode *parse_print_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PRINT_STMT;

    Token *print_tok = g_current_token;
    if (strcmp(print_tok->value, "print") != 0) {
        fprintf(stderr, "Expected 'print'\n");
        exit(1);
    }
    advance_token();
    expect(TOKEN_LPAREN, "Expected '('");
    node->print_stmt.expr = parse_expression();
    expect(TOKEN_RPAREN, "Expected ')'");
    expect(TOKEN_SEMICOLON, "Expected ';'");

    return node;
}

static ASTNode *parse_if_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IF_STMT;

    expect(TOKEN_KW_IF, "Expected '--if'");
    node->if_stmt.condition = parse_expression();
    
    expect(TOKEN_LBRACE, "Expected '{'");
    ASTNode **then_body = malloc(sizeof(ASTNode*) * 100);
    int then_idx = 0;
    while (current_type() != TOKEN_RBRACE) {
        then_body[then_idx++] = parse_statement();
    }
    expect(TOKEN_RBRACE, "Expected '}'");
    node->if_stmt.then_body = then_body;
    node->if_stmt.then_count = then_idx;

    ASTNode **else_if_conds = malloc(sizeof(ASTNode*) * 100);
    ASTNode ***else_if_bodies = malloc(sizeof(ASTNode**) * 100);
    int *else_if_counts = malloc(sizeof(int) * 100);
    int ei_idx = 0;
    while (current_type() == TOKEN_KW_ELSIF) {
        advance_token();
        ASTNode *cond = parse_expression();
        expect(TOKEN_LBRACE, "Expected '{'");
        ASTNode **body = malloc(sizeof(ASTNode*) * 50);
        int body_idx = 0;
        while (current_type() != TOKEN_RBRACE) {
            body[body_idx++] = parse_statement();
        }
        expect(TOKEN_RBRACE, "Expected '}'");
        else_if_conds[ei_idx] = cond;
        else_if_bodies[ei_idx] = body;
        else_if_counts[ei_idx] = body_idx;
        ei_idx++;
    }
    node->if_stmt.else_ifs_conds = else_if_conds;
    node->if_stmt.else_ifs_bodies = else_if_bodies;
    node->if_stmt.else_ifs_counts = else_if_counts;
    node->if_stmt.else_if_count = ei_idx;

    if (current_type() == TOKEN_KW_ELSE) {
        advance_token();
        expect(TOKEN_LBRACE, "Expected '{'");
        ASTNode **else_body = malloc(sizeof(ASTNode*) * 100);
        int else_idx = 0;
        while (current_type() != TOKEN_RBRACE) {
            else_body[else_idx++] = parse_statement();
        }
        expect(TOKEN_RBRACE, "Expected '}'");
        node->if_stmt.else_body = else_body;
        node->if_stmt.else_count = else_idx;
    } else {
        node->if_stmt.else_body = NULL;
        node->if_stmt.else_count = 0;
    }

    return node;
}

static ASTNode *parse_while_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE_STMT;

    expect(TOKEN_KW_WHILE, "Expected '--while'");
    node->while_stmt.condition = parse_expression();
    
    expect(TOKEN_LBRACE, "Expected '{'");
    ASTNode **body = malloc(sizeof(ASTNode*) * 100);
    int body_idx = 0;
    while (current_type() != TOKEN_RBRACE) {
        body[body_idx++] = parse_statement();
    }
    expect(TOKEN_RBRACE, "Expected '}'");
    node->while_stmt.body = body;
    node->while_stmt.body_count = body_idx;

    return node;
}

static ASTNode *parse_foreach_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FOREACH_STMT;

    expect(TOKEN_KW_FOREACH, "Expected '--foreach'");
    Token *var_tok = g_current_token;
    char *var_name = strdup(var_tok->value);
    expect(TOKEN_IDENTIFIER, "Expected loop variable name");
    node->foreach_stmt.var_name = var_name;

    expect(TOKEN_KW_IN, "Expected 'in'");
    node->foreach_stmt.iterable = parse_expression();

    expect(TOKEN_LBRACE, "Expected '{'");
    ASTNode **body = malloc(sizeof(ASTNode*) * 100);
    int body_idx = 0;
    while (current_type() != TOKEN_RBRACE) {
        body[body_idx++] = parse_statement();
    }
    expect(TOKEN_RBRACE, "Expected '}'");
    node->foreach_stmt.body = body;
    node->foreach_stmt.body_count = body_idx;

    return node;
}

static ASTNode *parse_break_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BREAK_STMT;
    expect(TOKEN_KW_BREAK, "Expected '--break'");
    expect(TOKEN_SEMICOLON, "Expected ';'");
    return node;
}

static ASTNode *parse_continue_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_CONTINUE_STMT;
    expect(TOKEN_KW_CONTINUE, "Expected '--continue'");
    expect(TOKEN_SEMICOLON, "Expected ';'");
    return node;
}

static ASTNode *parse_log_stmt() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_LOG_STMT;

    expect(TOKEN_KW_LOG, "Expected '--log'");
    node->log_stmt.message = parse_expression();
    
    node->log_stmt.level = strdup("info");
    if (current_type() == TOKEN_KW_LEVEL) {
        advance_token();
        Token *level_tok = g_current_token;
        expect(TOKEN_IDENTIFIER, "Expected level name (info, warn, debug)");
        node->log_stmt.level = strdup(level_tok->value);
    }
    expect(TOKEN_SEMICOLON, "Expected ';'");
    return node;
}

static ASTNode *parse_mode_directive() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_MODE_DIRECTIVE;
    expect(TOKEN_KW_MODE, "Expected '--mode'");
    Token *mode_tok = g_current_token;
    if (current_type() != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected mode name (silent, debug, ultra)\n");
        exit(1);
    }
    node->mode_directive.mode = strdup(mode_tok->value);
    advance_token();
    return node;
}

static ASTNode *parse_statement() {
    if (current_type() == TOKEN_KW_INIT) {
        return parse_var_decl();
    } else if (current_type() == TOKEN_KW_CALL) {
        return parse_call_stmt();
    } else if (current_type() == TOKEN_KW_IF) {
        return parse_if_stmt();
    } else if (current_type() == TOKEN_KW_WHILE) {
        return parse_while_stmt();
    } else if (current_type() == TOKEN_KW_FOREACH) {
        return parse_foreach_stmt();
    } else if (current_type() == TOKEN_KW_BREAK) {
        return parse_break_stmt();
    } else if (current_type() == TOKEN_KW_CONTINUE) {
        return parse_continue_stmt();
    } else if (current_type() == TOKEN_KW_LOG) {
        return parse_log_stmt();
    } else if (current_type() == TOKEN_IDENTIFIER) {
        return parse_print_stmt();
    } else {
        fprintf(stderr, "Unexpected token at start of statement: %s\n",
                token_type_names[current_type()]);
        exit(1);
    }
}

ASTNode *parse_program(Lexer *lexer) {
    g_lexer = lexer;
    advance_token();

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    ASTNode **stmts = malloc(sizeof(ASTNode*) * 200);
    int idx = 0;

    if (current_type() == TOKEN_KW_MODE) {
        stmts[idx++] = parse_mode_directive();
    }

    while (current_type() != TOKEN_EOF) {
        stmts[idx++] = parse_statement();
    }
    node->program.statements = stmts;
    node->program.stmt_count = idx;
    return node;
}