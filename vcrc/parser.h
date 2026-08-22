#ifndef PARSER_H
#define PARSER_H

#define MAX_TOKENS 64
#define MAX_TOKENS_LEN 256

typedef struct {
    char *tokens[MAX_TOKENS];
    int count;
} ParsedLine;

int parse_line(const char *line, ParsedLine *parsed);

void free_parsed_line(ParsedLine *parsed);

#endif