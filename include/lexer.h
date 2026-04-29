#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQ,
    TOKEN_EQEQ,
    TOKEN_NOTEQ,
    TOKEN_LT,
    TOKEN_LTEQ,
    TOKEN_GT,
    TOKEN_GTEQ,
    TOKEN_DOT,
    TOKEN_KW_INIT,
    TOKEN_KW_TYPE,
    TOKEN_KW_VAR,
    TOKEN_KW_CONST,
    TOKEN_KW_FUNC,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_ELSIF,
    TOKEN_KW_WHILE,
    TOKEN_KW_FOREACH,
    TOKEN_KW_IN,
    TOKEN_KW_RETURN,
    TOKEN_KW_CALL,
    TOKEN_KW_LOG,
    TOKEN_KW_LEVEL,
    TOKEN_KW_MODE,
    TOKEN_KW_SWITCH,
    TOKEN_KW_CASE,
    TOKEN_KW_DEFAULT,
    TOKEN_KW_OBSERVE,
    TOKEN_KW_WEIGHTS,
    TOKEN_KW_AS,
    TOKEN_KW_IMPORT,
    TOKEN_TYPE_MAYBE,
    TOKEN_TYPE_ONCE,
    TOKEN_TYPE_WATCH,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

typedef struct {
    const char *source;
    int pos;
    int line;
    int column;
} Lexer;

extern const char *token_type_names[];

Lexer *lexer_new(const char *source);
Token *lexer_next(Lexer *lexer);
void lexer_free(Lexer *lexer);

#endif