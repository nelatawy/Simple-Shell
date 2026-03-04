#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tok_utils.h"

token to_token(char* src, int cnt, bool expandable, bool splittable){
    token tok;
    tok.str = malloc(sizeof(char) * (cnt + 1));
    strncpy(tok.str, src, cnt);
    tok.str[cnt] = '\0';
    tok.expandable = expandable;
    tok.splittable = splittable;
    return tok;
}

token empty_tok(){
    token tok = {.str = "\0", .expandable = false, .splittable = false};
    return tok;
}

char* copy_str(char *token) {
    int len = strlen(token);
    char *new_str = malloc(sizeof(char) * (len + 1)); // size + null term.
    if (!new_str) return NULL;
    strcpy(new_str, token);
    new_str[len] = '\0';
    return new_str;
}


token* tokenize_input(char* input ,int* tok_count){
    int length = 0;
    token* tokens = malloc(sizeof(token) * 10);

    int last_sep_idx = -1;
    int in_quotes = 0;
    char last_q_type = '\"';
    int char_count = 0;

    for (int i = 0; i < strlen(input); i++)
    {   
        char_count = i - last_sep_idx + 1; //number of chars from last_sep till this char (inclusive)
        char c = input[i];
        if (c == '\"' || c == '\''){   
            if (!in_quotes){ //new quoted block
                in_quotes = 1;
                last_q_type = c;
                if (last_sep_idx < i -1) //we should add as a token as well --> case aloha"hello" (we must add aloha)
                    tokens[length++] = to_token(&input[last_sep_idx + 1], char_count - 2, true, true); 
                    // it can be both like $var if var="cd .." (without quotes) then we should split it after tokenization
                    // and we need to be able to expand it from $var to cd ..
                last_sep_idx = i;
            }
            else if(c == last_q_type){ //closing quoted block
                tokens[length++] = to_token(&input[last_sep_idx + 1], char_count - 2, c == '\"', false);
                //if enclosed by quotes --> do not split , and only allow expansion with double quotes
                in_quotes = 0;
                last_sep_idx = i;
            }
        }

        else if (!in_quotes && c == ' ') //unquoted term
        {
            if(char_count - 2 >= 1) //must have at least a char between sep
                tokens[length++] = to_token(&input[last_sep_idx + 1], char_count - 2, true, true);
                // remove the enclosing seperators , and since it's unquoted allow for splitting after expansion
            
            last_sep_idx = i;
        }
    }
    if (in_quotes){ //quotes weren't closed
        free_tokens(tokens, length);
        return NULL;
    }
    int n = strlen(input);
    if (last_sep_idx < n - 1) //catch last untracked 'unquoted' block
        tokens[length++] = to_token(&input[last_sep_idx + 1], char_count - 1, true, true); // remove the leading seperator


    if(tok_count != NULL)
        *tok_count = length;

    return tokens;
}

token* split_tokens(token* tokens, int* tok_cnt){ 
    token* new_tokens = malloc(sizeof(token) * (*tok_cnt) * 20);
    int new_len = 0;
    for (int i = 0; i < *tok_cnt; i++)
    {   
        if (!tokens[i].splittable){
            token new_tok = {.str = copy_str(tokens[i].str), .expandable = tokens[i].expandable, .splittable = false};
            new_tokens[new_len++] = new_tok;
            continue;
        }
            
          
        int part_cnt;
        token* tok_parts = tokenize_input(tokens[i].str, &part_cnt);
        for (int j = 0; j < part_cnt; j++)
        {
            new_tokens[new_len++] = tok_parts[j]; 
        }
        free(tok_parts); //only the list but not individual tokens
        
    }
    new_tokens = realloc(new_tokens, (new_len) * sizeof(token)); //trim the size back --> no over-allocation
    *tok_cnt = new_len;
    printf("%d is the new length", *tok_cnt);
    return new_tokens;
}

char** tok_to_str(token* toks, int cnt){
    char** strs = malloc(sizeof(char*) * (cnt + 1));
    for (int i = 0; i < cnt; i++)
    {
        strs[i] = copy_str(toks[i].str);
    }
    strs[cnt] = NULL;
    return strs;
}

void free_tokens(token* tokens, int tok_count) {
    for(int i = 0; i < tok_count; i++){
        free(tokens[i].str);
    }
    free(tokens);
}
