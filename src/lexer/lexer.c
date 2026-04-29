#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char *token_type_names[] = {
    "EOF", "IDENTIFIER", "STRING", "INT", "FLOAT",
    "LBRACE", "RBRACE", "LBRACKET", "RBRACKET", "LPAREN", "RPAREN",
    "COMMA", "COLON", "SEMICOLON", "PLUS", "MINUS", "STAR", "SLASH",
    "EQ", "EQEQ", "NOTEQ", "LT", "LTEQ", "GT", "GTEQ", "DOT",
    "KW_INIT", "KW_TYPE", "KW_VAR", "KW_CONST", "KW_FUNC",
    "KW_IF", "KW_ELSE", "KW_ELSIF", "KW_WHILE", "KW_FOREACH", "KW_IN",
    "KW_RETURN", "KW_CALL", "KW_LOG", "KW_LEVEL", "KW_MODE", "KW_SWITCH",
    "KW_CASE", "KW_DEFAULT", "KW_OBSERVE", "KW_WEIGHTS", "KW_AS",
    "KW_IMPORT", "TYPE_MAYBE", "TYPE_ONCE", "TYPE_WATCH",
    "KW_BREAK", "KW_CONTINUE",
    "TRUE", "FALSE", "NULL", "ERROR"
};

Lexer *lexer_new(const char *source) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

void lexer_free(Lexer *lexer) {
    free(lexer);
}

static void lexer_skip_whitespace(Lexer *lexer) {
    while (isspace(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '\r') {
        if (lexer->source[lexer->pos] == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->pos++;
    }
}

static void lexer_skip_comment(Lexer *lexer) {
    if (lexer->source[lexer->pos] == '#' && lexer->source[lexer->pos+1] == '#') {
        lexer->pos += 2;
        lexer->column += 2;
        while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '\n') {
            lexer->pos++;
            lexer->column++;
        }
    }
}

Token *lexer_next(Lexer *lexer) {
    while (1) {
        lexer_skip_whitespace(lexer);
        lexer_skip_comment(lexer);
        if (!isspace(lexer->source[lexer->pos]) && 
            !(lexer->source[lexer->pos] == '#' && lexer->source[lexer->pos+1] == '#')) {
            break;
        }
    }

    Token *token = malloc(sizeof(Token));
    token->line = lexer->line;
    token->column = lexer->column;
    token->value = NULL;

    char c = lexer->source[lexer->pos];
    if (c == '\0') {
        token->type = TOKEN_EOF;
        return token;
    }

    // Double-character tokens (--keywords)
    if (c == '-' && lexer->source[lexer->pos+1] == '-') {
        lexer->pos += 2;
        lexer->column += 2;
        int start = lexer->pos;
        while (isalpha(lexer->source[lexer->pos])) {
            lexer->pos++;
            lexer->column++;
        }
        int len = lexer->pos - start;
        char *word = malloc(len + 1);
        strncpy(word, lexer->source + start, len);
        word[len] = '\0';

        if (strcmp(word, "init") == 0) token->type = TOKEN_KW_INIT;
        else if (strcmp(word, "type") == 0) token->type = TOKEN_KW_TYPE;
        else if (strcmp(word, "if") == 0) token->type = TOKEN_KW_IF;
        else if (strcmp(word, "else") == 0) token->type = TOKEN_KW_ELSE;
        else if (strcmp(word, "elsif") == 0) token->type = TOKEN_KW_ELSIF;
        else if (strcmp(word, "while") == 0) token->type = TOKEN_KW_WHILE;
        else if (strcmp(word, "foreach") == 0) token->type = TOKEN_KW_FOREACH;
        else if (strcmp(word, "return") == 0) token->type = TOKEN_KW_RETURN;
        else if (strcmp(word, "call") == 0) token->type = TOKEN_KW_CALL;
        else if (strcmp(word, "log") == 0) token->type = TOKEN_KW_LOG;
        else if (strcmp(word, "level") == 0) token->type = TOKEN_KW_LEVEL;
        else if (strcmp(word, "mode") == 0) token->type = TOKEN_KW_MODE;
        else if (strcmp(word, "switch") == 0) token->type = TOKEN_KW_SWITCH;
        else if (strcmp(word, "case") == 0) token->type = TOKEN_KW_CASE;
        else if (strcmp(word, "default") == 0) token->type = TOKEN_KW_DEFAULT;
        else if (strcmp(word, "observe") == 0) token->type = TOKEN_KW_OBSERVE;
        else if (strcmp(word, "weights") == 0) token->type = TOKEN_KW_WEIGHTS;
        else if (strcmp(word, "as") == 0) token->type = TOKEN_KW_AS;
        else if (strcmp(word, "import") == 0) token->type = TOKEN_KW_IMPORT;
        else if (strcmp(word, "break") == 0) token->type = TOKEN_KW_BREAK;
        else if (strcmp(word, "continue") == 0) token->type = TOKEN_KW_CONTINUE;
        else token->type = TOKEN_ERROR;
        free(word);
        return token;
    }

    // Strings
    if (c == '\'' || c == '"') {
        char quote = c;
        lexer->pos++;
        lexer->column++;
        int start = lexer->pos;
        while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != quote) {
            if (lexer->source[lexer->pos] == '\\') lexer->pos++;
            lexer->pos++;
            lexer->column++;
        }
        int len = lexer->pos - start;
        token->value = malloc(len + 1);
        strncpy(token->value, lexer->source + start, len);
        token->value[len] = '\0';
        token->type = TOKEN_STRING;
        lexer->pos++;
        lexer->column++;
        return token;
    }

    // Numbers
    if (isdigit(c)) {
        int start = lexer->pos;
        while (isdigit(lexer->source[lexer->pos])) {
            lexer->pos++;
            lexer->column++;
        }
        if (lexer->source[lexer->pos] == '.') {
            lexer->pos++;
            lexer->column++;
            while (isdigit(lexer->source[lexer->pos])) {
                lexer->pos++;
                lexer->column++;
            }
            token->type = TOKEN_FLOAT;
        } else {
            token->type = TOKEN_INT;
        }
        int len = lexer->pos - start;
        token->value = malloc(len + 1);
        strncpy(token->value, lexer->source + start, len);
        token->value[len] = '\0';
        return token;
    }

    // Identifiers and keywords
    if (isalpha(c) || c == '_') {
        int start = lexer->pos;
        while (isalnum(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_') {
            lexer->pos++;
            lexer->column++;
        }
        int len = lexer->pos - start;
        char *word = malloc(len + 1);
        strncpy(word, lexer->source + start, len);
        word[len] = '\0';

        if (strcmp(word, "var") == 0) {
            token->type = TOKEN_KW_VAR;
            token->value = word;
        } else if (strcmp(word, "const") == 0) {
            token->type = TOKEN_KW_CONST;
            token->value = word;
        } else if (strcmp(word, "func") == 0) {
            token->type = TOKEN_KW_FUNC;
            token->value = word;
        } else if (strcmp(word, "maybe") == 0) {
            token->type = TOKEN_TYPE_MAYBE;
            token->value = word;
        } else if (strcmp(word, "once") == 0) {
            token->type = TOKEN_TYPE_ONCE;
            token->value = word;
        } else if (strcmp(word, "watch") == 0) {
            token->type = TOKEN_TYPE_WATCH;
            token->value = word;
        } else if (strcmp(word, "in") == 0) {
            token->type = TOKEN_KW_IN;
            free(word);
        } else if (strcmp(word, "true") == 0) {
            token->type = TOKEN_TRUE;
            free(word);
        } else if (strcmp(word, "false") == 0) {
            token->type = TOKEN_FALSE;
            free(word);
        } else if (strcmp(word, "null") == 0) {
            token->type = TOKEN_NULL;
            free(word);
        } else {
            token->type = TOKEN_IDENTIFIER;
            token->value = word;
        }
        return token;
    }

    // Single-character tokens
    lexer->pos++;
    lexer->column++;
    switch (c) {
        case '{': token->type = TOKEN_LBRACE; break;
        case '}': token->type = TOKEN_RBRACE; break;
        case '[': token->type = TOKEN_LBRACKET; break;
        case ']': token->type = TOKEN_RBRACKET; break;
        case '(': token->type = TOKEN_LPAREN; break;
        case ')': token->type = TOKEN_RPAREN; break;
        case ',': token->type = TOKEN_COMMA; break;
        case ':': token->type = TOKEN_COLON; break;
        case ';': token->type = TOKEN_SEMICOLON; break;
        case '+': token->type = TOKEN_PLUS; break;
        case '-': token->type = TOKEN_MINUS; break;
        case '*': token->type = TOKEN_STAR; break;
        case '/': token->type = TOKEN_SLASH; break;
        case '.': token->type = TOKEN_DOT; break;
        case '=': 
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->column++;
                token->type = TOKEN_EQEQ;
            } else token->type = TOKEN_EQ;
            break;
        case '!':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->column++;
                token->type = TOKEN_NOTEQ;
            } else token->type = TOKEN_ERROR;
            break;
        case '<':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->column++;
                token->type = TOKEN_LTEQ;
            } else token->type = TOKEN_LT;
            break;
        case '>':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->column++;
                token->type = TOKEN_GTEQ;
            } else token->type = TOKEN_GT;
            break;
        default: token->type = TOKEN_ERROR;
    }
    return token;
}