#ifndef TOK_UTILS_H
#define TOK_UTILS_H

#include <stdbool.h>

typedef struct{
    char *str;
    bool splittable;
    bool expandable;
} token;

token to_token(char* src, int cnt, bool expandable, bool splittable);

token empty_tok();

char* copy_str(char *token);

token* tokenize_input(char* input ,int* tok_count);

token* split_tokens(token* tokens, int* tok_cnt);

char** tok_to_str(token* toks, int cnt);

void free_tokens(token* tokens, int tok_count);

void free_strs(char** strs, int cnt);
#endif

