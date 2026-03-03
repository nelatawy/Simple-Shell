#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tok_utils.h"

void allocate_n_copy(char **dest, char* src, int cnt){
    *dest = malloc((cnt + 1) * sizeof(char));
    strncpy(*dest, src, cnt);
    (*dest)[cnt] ='\0';    
}

char** tokenize_input(char *input ,int* tok_count){
    int length = 0;
    char**tokens = malloc(sizeof(char*) * 10);

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
                    allocate_n_copy(&tokens[length++], &input[last_sep_idx + 1], char_count - 2);
                last_sep_idx = i;
            }
            else if(c == last_q_type){ //closing quoted block
                allocate_n_copy(&tokens[length], &input[last_sep_idx], char_count); //to include the quotes for later semantics
                length++;
                in_quotes = 0;
                last_sep_idx = i;
            }
        }

        else if (!in_quotes && c == ' ')
        {
            if(char_count >= 1){
                allocate_n_copy(&tokens[length], &input[last_sep_idx + 1], char_count - 2); // remove the enclosing spaces
                length++;
            }
            last_sep_idx = i;
        }
    }
    if (in_quotes){ //quotes weren't closed
        free_tokens(tokens, length);
        return NULL;
    }
    int n = strlen(input);
    if (last_sep_idx < n - 1){ //catch last untracked block
        allocate_n_copy(&tokens[length], &input[last_sep_idx + 1], char_count - 1); // remove the leading space
        length++;
    }

    if(tok_count != NULL)
        *tok_count = length;

    return tokens;
}

char** split_tokens(char **tokens, int tok_cnt){ 
    char **new_tokens = malloc(sizeof(char*) * tok_cnt * 20);
    int new_len = 0;
    for (int i = 0; i < tok_cnt; i++)
    {   
        int part_cnt;
        char **tok_parts = tokenize_input(tokens[i], &part_cnt);
        for (int j = 0; j < part_cnt; j++)
        {
            new_tokens[new_len++] = tok_parts[j]; //no need to strcpy --> just take the out strings
        }
        free(tok_parts); //only the list but not the strings
        
    }
    new_tokens = realloc(new_tokens, (new_len + 1) * sizeof(char*)); //trim the size back --> no over-allocation
    // execvp expects argv to be NULL-terminated
    new_tokens[new_len] = NULL;
    return new_tokens;
}

void free_tokens(char** tokens, int tok_count) {
    for(int i = 0; i < tok_count; i++){
        free(tokens[i]);
    }
    free(tokens);
}
