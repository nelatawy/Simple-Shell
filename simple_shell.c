#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <termios.h>

#include "env_utils.h"
#include "tok_utils.h"
#include "commands.h"


env_var *vars;
int def_vars_cnt;
int var_limit = 100;

struct termios og_config;

void run_shell();
void handle_child_exit(int signum);
void setup_env();
void run_startup_code();

void enable_raw_mode(){
    tcgetattr(STDIN_FILENO, &og_config);
    struct termios raw = og_config;
    raw.c_lflag &= ~(ECHO | SIGINT | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &og_config);
}

void handle_move_right(char * input, int* pos){
    if (*pos < strlen(input))
       *pos = *pos + 1;
    
    
    printf("\x1b[1C");
    fflush(stdout);   
}

void handle_move_left(char * input, int* pos){
    if (*pos > 0)
       *pos = *pos - 1;
    
    
    printf("\x1b[1D");
    fflush(stdout);   
}

void handle_backspace(char * input, int* pos){
    if (*pos <= 0) 
        return;
    *pos = *pos - 1;
    strcpy(input + *pos, input + *pos + 1); // remove char

    printf("\x1b[1D");

    printf("%s ", input + *pos);

    printf("\x1b[%dD", strlen(input) - *pos);

    fflush(stdout);
    
}

void handle_insert_char(char * input, char c, int* pos){
    insert_char(input, c, *pos);   // shift buffer and insert
    printf("%s", input + *pos);       // print the inserted char + rest of line
    int move_back = strlen(input) - (*pos + 1);
    if (move_back > 0)
        printf("\x1b[%dD", move_back); // move cursor back after inserted char
    *pos = *pos + 1;
    fflush(stdout);
}
int main() {
    signal(SIGCHLD, handle_child_exit);
    init_vars_array();
    run_startup_code();
    setup_env();
    run_shell();
}


void handle_child_exit(int signum){
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0);
    //handles if many children exit at once
    
}

void setup_env(){
    // char *home = getenv("HOME");

    if (chdir("/") != 0) {
        perror("chdir failed");
        return ;
    }

}

void run_shell(){
    char *usr = get_expanded_str("USER",4);
    printf("\n\033[1;34mWelcome %s\033[0m ", usr);
    free(usr);


    while (1){
        char * cwd = getcwd(NULL,0);
        printf("\n\033[1;32m%s\033[0m :", cwd);  
        fflush(stdout);
        free(cwd);

        char *input = malloc(sizeof(char) * 200);
        input[0] = '\0';
        // scanf(" %199[^\n]", input);
        int pos = 0;
        enable_raw_mode();
        while (1){
            char c[1];
            read(STDIN_FILENO, &c, 1);
            if (c[0] == 3) {//CTRL + C
                disable_raw_mode();
                exit(0);
            }

            else if (c[0] == 27) //ESC
            {
                char seq[2];
                read(STDIN_FILENO, &seq[0], 1);
                read(STDIN_FILENO, &seq[1], 1);

                if (seq[0] == '[') {
                    if (seq[1] == 'A') ; // top
                    if (seq[1] == 'B') ; // bottom
                    if (seq[1] == 'C') handle_move_right(input, &pos);// right
                    if (seq[1] == 'D') handle_move_left(input, &pos); // left
                }
            }
            else if (c[0] == 127) //backspace
            {
                handle_backspace(input, &pos);
            }

            else if (c[0] == '\n'){
                break;
            }
            
            else {
                handle_insert_char(input, c[0], &pos);
            }
                
        }
        disable_raw_mode();
        if (input !=NULL)
        {   
            printf("\n");
            exec_command(input);
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


