#include <ctype.h>
#include <fcntl.h>
#include "redirection.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "swish.h"
#include "wolfie.h"
void change_directory(char *path);

int check_key_press(char **c, int position);

void execute_cmd(command *cmd, int a);

char* get_cur_wrk_dir(void);

void get_input(void);

void initialize_cmd(command *cmd, char **token_array);

int isalnumvar(char *str);
int built_in(command * cmd, int ac);

int input_file(char *argv[], command * cmd);
int is_redir_symbol(char c);

void left_shift_cmd(int shift_amt);

void print_prompt(char *prompt);

void print_tokens(char **tok_arr);

void print_working_dir(void);

int set_variable(char ** var_value, int a);

int tokenize_input(char *input_str, char ***token_array, int n_tok_arr, char *delimiters);

void valid_var(char **args_value, int ar);

char prompt[MAX_PATH_LENGTH]; /* prompt to appear at the beginning of every line in SWISH */

int file_exists(char **file_path, struct stat a);