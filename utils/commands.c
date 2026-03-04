#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include "commands.h"
#include "env_utils.h"
#include "tok_utils.h"



command builtins[] = {
    {"cd", handle_cd},
    {"exit", handle_exit},
    {"export", handle_export},
    {"echo", handle_echo},
    {"eval", handle_eval},
    {"clear", handle_clear}
};


static char *extract_from_quotes(char* str){
    int len = strlen(str);
    int has_quotes = 1;
    int start = 1;
    if (str[0] != '\'' || str[len -1] != '\'')
    {
       has_quotes = 0;
       start = 0;
    }
    
    int char_cnt = len - ((has_quotes)?2:0);
    char *inner = malloc(sizeof(char) * (char_cnt + 1));
    strncpy(inner, &str[start], char_cnt);
    inner[char_cnt] = '\0';
    return inner;
}

static int handle_unitoken_assignment(char* assignment){
    char *var_name;
    char *val;
    int len = strlen(assignment);

    for (int i = 0; i < len; i++){
        if (assignment[i] !='=')
            continue;

        var_name = malloc(sizeof(char)*(i + 1));
        strncpy(var_name, assignment, i);
        var_name[i] = '\0';
        if (!is_valid_var(var_name))
        {   free(var_name);
            perror("a variable name must start with an alphabetical letter and the rest must be alphaneumeric");
            return 0;
        }

        val = extract_from_quotes(&assignment[i + 1]);

        s_env(var_name, val);
        return 1;
    }
    return 0;
}

static int handle_bitok_assignment(char* first, char* second){
    char *var_name;
    char *val;
    int len1 = strlen(first);
    int len2 = strlen(second);
    first[len1 - 1] = '\0'; //remove the equal
    if (!is_valid_var(first))
    {
        perror("a variable name must start with an alphabetical letter and the rest must be alphaneumeric");
        return 0;
    }
    var_name = malloc(sizeof(char)*(len1));
    strcpy(var_name, first);
    val = extract_from_quotes(second);
    s_env(var_name, val);
    return 1;
}


int is_valid_var(char *var){
    int n = strlen(var);

    if (!isalpha(var[0]))
        return 0;
    
    for (int i = 1; i < n; i++)
    {
        if(!isalnum(var[i]))
            return 0;
    }
    return 1;
    
}

int handle_cd(int argc, char** argv){
    int res;
    if (argc == 1) //only the file
    {
        res = chdir("/");
    }
    else if (strcmp(argv[1], "~")==0)
    {
        char *home_path = getenv("HOME");
        res =chdir(home_path);
    }else{
        res =  chdir(argv[1]);
    }

    return res;
}

int handle_exit(int argc, char** argv){
    exit(0);
}

int handle_export(int argc, char** argv){
    char *assignment = argv[1];
    int len = strlen(assignment);
    int success = 0;

    if (assignment[len - 1] == '=') //then the value is in the next var
    {
        success = handle_bitok_assignment(argv[1], argv[2]);
    } else {
        success = handle_unitoken_assignment(assignment);
    }


    if (success)
        return 1;
    perror("invalid syntax,should be [Var]=[val]");
    return 0;
    
}

int handle_echo(int argc, char **argv){
    for (int i = 1; i < argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return 0;
    
}

int handle_clear(int argc, char **argv){
    printf("\033[2J\033[H");
    fflush(stdout);
    return 0;
}

int handle_eval(int argc, char **argv){
    exec_command(argv[1]);
}

void exec_command(char *input){
    int tok_count;
    token* tokens = tokenize_input(input, &tok_count);

    
    if(tokens == NULL){
        perror("An error occured while trying to parse command");
        return;
    }
    expand_all_in_tokens(tokens, tok_count);


    token* temp = tokens;
    int old_tk_cnt = tok_count;


    tokens = split_tokens(tokens, &tok_count);

    free_tokens(temp, old_tk_cnt);

    char **argv = tok_to_str(tokens, tok_count);
    free_tokens(tokens, tok_count);

    if (is_builtin(argv[0])){
        handle_builtin(tok_count, argv);
    } else {
        handle_external_commands(tok_count, argv);
    }
    free_strs(argv, tok_count);
    
}

void handle_builtin(int argc, char** argv){
    int len = sizeof(builtins)/sizeof(builtins[0]);
    for (int i = 0; i < len; i++){
        if (strcmp(builtins[i].name, argv[0]) == 0)
        {
            builtins[i].func(argc, argv);
        }
    }
}

void handle_external_commands(int argc, char **argv){
    int in_bg = (argc > 1 && strcmp(argv[1],"&") == 0);
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(argv[0], argv);
    } else if(!in_bg){ //parent should wait
        int stats = 0;
        waitpid(pid, &stats, 0);
    }
}


int is_builtin(char *name){
    int len = sizeof(builtins)/sizeof(builtins[0]);
    for (int i = 0; i < len; i++){
        if (strcmp(builtins[i].name, name) == 0)
            return 1;
    }
    return 0;
}