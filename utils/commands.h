
typedef struct command{
    char * name;
    int (*func)(int,char**);
} command;


int is_valid_var(char *var);

int handle_cd(int argc, char** argv);

int handle_exit(int argc, char** argv);

int handle_export(int argc, char** argv);

int handle_echo(int argc, char **argv);

int handle_eval(int argc, char **input);

int handle_clear(int argc, char **argv);

void exec_command(char *input);

void handle_builtin(int argc, char** argv);

void handle_external_commands(int argc, char **argv);

int is_builtin(char *name);