#ifndef TOK_UTILS_H
#define TOK_UTILS_H

#include "env_utils.h"
extern env_var *vars;
extern int def_vars_cnt;

void allocate_n_copy(char **dest, char* src, int cnt);

char** tokenize_input(char *input ,int* tok_count);

char** split_tokens(char **tokens, int tok_cnt);

void free_tokens(char** tokens, int tok_count);

#endif

