#ifndef ADVENTURE_PARSER_H
#define ADVENTURE_PARSER_H

#include <stddef.h>

#include "advent.h"

enum parse_status {
        PARSE_OK = 0,
        PARSE_TOO_MANY_WORDS,
};

int adventure_parse_command(const char *line, char tokens[2][WORDSIZE], size_t *count);
int adventure_canonicalize_token(const char *input, char *output, size_t size);
const char *adventure_two_word_error(void);

#endif /* ADVENTURE_PARSER_H */
