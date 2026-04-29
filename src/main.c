#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: kore <file.k>\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) { perror("fopen"); return 1; }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *source = malloc(length + 1);
    fread(source, 1, length, file);
    source[length] = '\0';
    fclose(file);

    Lexer *lexer = lexer_new(source);
    ASTNode *program = parse_program(lexer);
    interpret(program);

    ast_free(program);
    lexer_free(lexer);
    free(source);
    return 0;
}