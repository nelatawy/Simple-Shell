# Simple-Shell
### A simple shell that mimics the bash shell, having it's own env and .rc file and some implementations of builtin commands

## Startup
Run the following command in the Repo directory to compile all into one program and add the `utils` dir to the `include` path.
```bash
gcc simple_shell.c utils/*.c  -Iutils -o shell
```
---

## Main Module
The facade from which the basic abstract functionality of shell can be viewed and examined is in the `simple_shell.c` file, let's break it down.
### Main function
```c
int main() {
    signal(SIGCHLD, handle_child_exit);
    init_vars_array();
    run_startup_code();
    setup_env();
    run_shell();
}
```
the function defines the sequence of operations the shell goes through on startup
1. It defines the interrupt subroutine that will be triggered once a child process is killed or exits (to avoid having zombies).
2. It initializes the array that should carry all the env variables (to be switched to a hashmap later on).
3. It runs the code snippet defined in the `.shellrc` file (providing similar functionality to `.zshrc` or `.bashrc`).
4. It sets the working directory to be the `root`.
5. And finally starts the shell for the `user` to interact with.

### Child process interrupt handler
```c
void handle_child_exit(int signum){
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0);
    //handles if many children exit at once
    
}
```
the reason why it's a `while` while rather than a simple `waitpid`is because the OS doesnt't queue the signals, so if two processes exit at once maybe only 1 interrupt is triggered so only one of them is reaped, so this handles such a case, and we are setting the `WNOHANG` bit to HIGH, this handles the case of no child-processes were killed yet, so it makes sure the function call is non-blocking.

### Running the shell
```c
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

```
the first part is merely for aesthetics, it gets the `USER` env that can be defined in the `.shellrc` to give a personalized feel to the shell.

then we enter a `while` that simply gets the input from the stdin, prints the `cwd` (preceeded by an escape sequence to color the terminal and highlight the cwd).
i chose `termios` to be able to have control over the effect of cursor keys and have the ability to go back and forth and remove some chars and have a more user-friendly experience (it's a must to be able to edit your command tbh).
we simply enable raw-mode to have full control over the stdin and let be a simple dumb buffer and disabling the canonical behavior to make it char-buffered not line-buffered and we provide custom behavior according to the key pressed.

and then we check if input is not NULL and then we start handling the command.

---
# Utils
## Command Utils
It handles all the operations related to executing commands and defining the builtin commands' behavior.
```c
command builtins[] = {
    {"cd", handle_cd},
    {"exit", handle_exit},
    {"export", handle_export},
    {"echo", handle_echo},
    {"eval", handle_eval},
    {"clear", handle_clear}
};
```
we define an array containing all the command_name and pointers to the handler function of each for easy addition of builtin commands.

### Executing a command
```c
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
```
this function defines the steps to handle the the input and execute the appropriate command,it goes as follows:
#### 1. First we tokenize the input either by being enclosed by quotes, or spaces (while keeping information about each token).
#### 2. We expand each token (after checking if it's expandable or not) by substituting env variables by their values.
#### 3. After expansion we check if a token is splittable, then we perform a word split 
*(to make cases like `ls $x` when` x="-l -a"` work by treating `$x` as multiple seperate tokens after expansion).*

and then we decide how to deal with the command depending on wether or not it's a builtin command or an external one.

### Handling builtin commands
```c
void handle_builtin(int argc, char** argv){
    int len = sizeof(builtins)/sizeof(builtins[0]);
    for (int i = 0; i < len; i++){
        if (strcmp(builtins[i].name, argv[0]) == 0)
        {
            builtins[i].func(argc, argv);
        }
    }
}
```
we simply loop over the defined builtins and look for the command, and if found we call the function whose pointer is associated with the defined command.

### Handling external commands
```c
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
```
here we handle background processes as well, if the 2nd argument is `&` then it's a background process, so if it's a background process the parent doesn't wait for the child-process to exit and simply continues.

### Builtin command handlers
```c
int handle_cd(int argc, char** argv);

int handle_exit(int argc, char** argv);

int handle_export(int argc, char** argv);

int handle_echo(int argc, char **argv);

int handle_eval(int argc, char **input);

int handle_clear(int argc, char **argv);
```
for every builtin command we have a handler with a similar signature `int func(int, char**)` and this seperation makes the logic modular and easier to both reason about, and extend.

### Auxilary
There are also some checking logic like `is_valid_var(char*)` that checks if a variable in a command like `export` follows the rules of a variable (starting with letter) and all chars are alphaneumeric.

---
## Tokenization Utils
### Core tokenization logic

```c
token* tokenize_input(char* input ,int* tok_count){...}
```
I recommend checking the method there `utils/tok_utils.c` it is well-documented but it could clutter the Readme
the function simply flows like this:
It iterates over all chars of the input and if:
1. char is a quotations it checks wether it matches the starting outer quotations and if so adds this entire block as a token, and if we are not in a quoted block already then we consider this a start of a quoted block, else we just keep going even if we have a quote but of a different type (to allow nesting).
2. if outside of a quoted block and char is not a quotation then we look for spaces, and we get all char sequences between spaces and we consider them as tokens.


### Splitting tokens after expansion
```c
token* split_tokens(token* tokens, int* tok_cnt){ 
    token* new_tokens = malloc(sizeof(token) * (*tok_cnt) * 20);
    int new_len = 0;
    for (int i = 0; i < *tok_cnt; i++)
    {   
        if (!tokens[i].splittable){ // we simply add the token as is
            token new_tok = {.str = copy_str(tokens[i].str), .expandable = tokens[i].expandable, .splittable = false};
            new_tokens[new_len++] = new_tok;
            continue;
        }

        int part_cnt;
        token* tok_parts = tokenize_input(tokens[i].str, &part_cnt); //we split to tokens
        for (int j = 0; j < part_cnt; j++)
        {
            new_tokens[new_len++] = tok_parts[j];
            // it copies the struct with all it's parts (include the pointer to allocated string)
        }
        free(tok_parts); //old array with old copy of tokens are freed but the new copies are intact
        
    }
    new_tokens = realloc(new_tokens, (new_len) * sizeof(token)); //trim the size back --> no over-allocation
    *tok_cnt = new_len;
    return new_tokens;
}
```
this step is critical to have the 'shell-standard' parsing logic and be able to bind an env to a command like or have more robust and powerful bindings than before.
```
export x="cd .."
$x
```
without the splitting the $x will be expanded to "cd .." as a single argument which is incorrect.

### Auxilary
```c
token to_token(char* src, int cnt, bool expandable, bool splittable);

token empty_tok();

char* copy_str(char *token);

void free_tokens(token* tokens, int tok_count);

void free_strs(char** strs, int cnt);

char** tok_to_str(token* toks, int cnt);
```
these functions provide needed functionality for the core logic to work, like freeing resources ,mapping between a token and it's string etc..

---
## Env Utils
### Core expansion logic
```c
token expand_in_token(token tok){...}
```
I also recommend checking this method out in `utils/env_utils.c`, i also excluded it from this README to reduce uneccessary clutter, but to simply describe this function, the sole purpose of it is to take a `token`, check if it's expandable or not and if it is, it starts processing it's chars one by one, adding all the char that are not part of an `env_var` name and once it encounters `$` it starts counting the rest as a variable name, but if another `$` follows it, it expands the `$$` to  `current_pid` like in bash, and returns the new `expanded` token.


### Handling the env vars
```c
void init_vars_array(){
    vars = malloc(INITIAL_VAR_CNT * sizeof(env_var));
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
```
these are the core of the env vars logic, they handle initialization of the array, expanding it if we hit the limit, and setting a new env variable (or updating it's value if it was already bound).