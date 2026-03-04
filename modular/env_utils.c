#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "env_utils.h"

#define INITIAL_VAR_CNT 10


void init_vars_array(){
    vars = malloc(INITIAL_VAR_CNT * sizeof(env_var));
    s_env("hello", "world");
}

void s_env(char *var_name, char *val){
    for (int i = 0; i < def_vars_cnt; i++)
    {
        if (strcmp(var_name, vars[i].var_name) != 0){
            continue;
        }
        vars[i].value = realloc(vars[i].value, (strlen(val) + 1) * sizeof(char));
        strcpy(vars[i].value, val);
        vars[i].value[strlen(val)] = '\0';
        return;
    }
    
    if(def_vars_cnt == var_limit){
        expand_vars_array(); 
    }
    env_var var = {var_name, val};
    vars[def_vars_cnt++] = var; 
}

void expand_vars_array(){
    env_var *new_arr = malloc((int)(var_limit * 1.5) * sizeof(env_var));
    if (new_arr == NULL)
    {
        perror("failed to expand memory to accomodate new env_var");
        return;
    }
    memcpy(new_arr, vars, sizeof(env_var) * def_vars_cnt);
    free(vars);
    vars = new_arr;
    var_limit *= 1.5;
}

char* get_expanded_str(char *token, int cnt){
    for (int i = 0; i < def_vars_cnt; i++){
        if(cnt != strlen(vars[i].var_name))
            continue;
        if(strncmp(token, vars[i].var_name, cnt) == 0){
            char *value = malloc(sizeof(char) * (strlen(vars[i].value) + 1));
            if (value == NULL) return NULL;
            strcpy(value, vars[i].value);
            return value;
        }
    }
    return NULL;
}

void append_expanded_val(char *new_str, char *token, int cnt){
    char* value = get_expanded_str(token, cnt);
    if (value){
        strcat(new_str, value);
        free(value);
    }
}

// static char* handle_double_quotes(char *token) {
//     int len = strlen(token);

//     // char **tokens = tokenize_input(inner, &tok_cnt);
//     // free(inner);
//     // if (!tokens) return NULL;

//     char* expanded = expand_in_token(token);

//     int total_size = 0;
//     // for (int i = 0; i < tok_cnt; i++)
//     //     total_size += strlen(tokens[i]);
    
//     // char *new_str = malloc(sizeof(char) * (total_size + tok_cnt));
//     // if (!new_str) {
//     //     free_tokens(tokens, tok_cnt);
//     //     return NULL;
//     // }
//     // new_str[0] = '\0';

//     // for (int i = 0; i < tok_cnt; i++) {
//     //     strcat(new_str, tokens[i]);
//     //     if (i < tok_cnt - 1) {
//     //         strcat(new_str, " ");
//     //     }
//     // }
//     // free_tokens(tokens, tok_cnt);
//     char *new_str = malloc(sizeof(char) * (len + 1)); // size + null term.
//     if (!new_str) return NULL;
//     strcpy(new_str, token);
//     new_str[len] = '\0';
//     return new_str;
// }

token expand_in_token(token tok){
    if (tok.str == NULL || strlen(tok.str) == 0) return empty_tok();
    if (!tok.expandable){
        token new_tok = {.str = copy_str(tok.str), .expandable = false, .splittable = tok.splittable};
        return new_tok;
    }

    int end = strlen(tok.str) - 1;
    char *new_str = malloc(sizeof(char) * 1024);
    if (!new_str) return empty_tok();
    new_str[0] = '\0';

    int last_var_start = -1;
    int in_var = 0;
    int var_symb_cnt = 0;
    int new_str_len = 0;

    for(int i = 0; i <= end; i++){     
        if (tok.str[i] == '$'){
            var_symb_cnt++;
            last_var_start = i;
            in_var = 1;
        }
        
        if (var_symb_cnt == 2) {
            char pid_buffer [10];
            sprintf(pid_buffer, "%d", (int) getpid());
            strcat(new_str, pid_buffer);
            var_symb_cnt = 0;
            in_var = 0;
            new_str_len = strlen(new_str);
        }
        else if(!in_var) {
            new_str[new_str_len++] = tok.str[i];
            new_str[new_str_len] = '\0';
        } 
        else if(tok.str[i] == ' '){
            append_expanded_val(new_str, &tok.str[last_var_start + 1], i - last_var_start - 1);
            new_str_len = strlen(new_str);
            in_var = 0;
            var_symb_cnt = 0;
        }
    }

    if (in_var){//the variabe wasn't marked yet
        append_expanded_val(new_str, &tok.str[last_var_start + 1], end - last_var_start);
        new_str_len = strlen(new_str);
    }

    new_str = realloc(new_str, new_str_len + 1);
    new_str[new_str_len] = '\0';

    token new_tok = {.str = new_str, .expandable = false, .splittable = tok.splittable};
    return new_tok;
}

void expand_all_in_tokens(token* tokens, int size){
    for (int i = 0; i < size; i++){
        token new_tok = expand_in_token(tokens[i]);
        free(tokens[i].str);  //it's fine since it's already copied in the new token
        tokens[i] = new_tok;
    }
}
