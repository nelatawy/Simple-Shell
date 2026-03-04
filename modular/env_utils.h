#ifndef ENV_UTILS_H
#define ENV_UTILS_H

#include "tok_utils.h"
#define INITIAL_VAR_CNT 10

typedef struct env_var{
    char *var_name;
    char *value;
} env_var;



extern env_var *vars;
extern int def_vars_cnt;
extern int var_limit;

void init_vars_array();

void s_env(char *var_name, char *val);

void expand_vars_array();

char* get_expanded_str(char *token, int cnt);

void append_expanded_val(char *new_str, char *token, int cnt);

token expand_in_token(token tok);

void expand_all_in_tokens(token* tokens, int size);



#endif