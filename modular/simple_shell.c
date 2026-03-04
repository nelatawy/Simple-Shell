#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

#include "env_utils.h"
#include "tok_utils.h"
#include "commands.h"


env_var *vars;
int def_vars_cnt;
int var_limit = 100;


void run_shell();
void handle_child_exit(int signum);
void setup_env();
void run_startup_code();



int main() {
    signal(SIGCHLD, handle_child_exit);
    run_startup_code();
    setup_env();
    run_shell();
}


void handle_child_exit(int signum){
    pid_t pid;
    while (pid = waitpid(-1, NULL, WNOHANG) > 0);
    //handles if many children exit at once
    
}

void setup_env(){
    char *home = getenv("HOME");

    if (chdir("/") != 0) {
        perror("chdir failed");
        return ;
    }

    init_vars_array();
}

void run_shell(){
    printf("\n\033[1;34m%s\033[0m ", "Welcome back R3tro");
    while (1){

        printf("\n\033[1;32m%s\033[0m :", getcwd(NULL,0));  

        char *input = malloc(sizeof(char) * 200);
        scanf(" %199[^\n]", input);
        exec_command(input);
        if (input !=NULL)
        {
            free(input);
        }
        
    }
}

void run_startup_code(){
    FILE* f = fopen(".shellrc","r");
    char buffer [300];
    if (f == NULL)
        return;

    while (fgets(buffer, 300, f))
    {
        exec_command(buffer);
    }
    
}



