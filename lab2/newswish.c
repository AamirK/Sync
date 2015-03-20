/* CSE 306: Sea Wolves Interactive SHell */

/**
 * Core assignment
 * Write a C program named "swish" (for SeaWolves Interactive SHell) that performs a subset
 * of commands you're familiar with from other shells like GNU's Bash.
 * You're welcome to study the code for bash, but the code you submit should be your own!
 * When you start your shell, you should be able to type commands such as this and see their output:
 *   $ ./swish
 *   swish> ls
 *   # output from ls
 *   swish> ls -l
 *   # verbose output from ls
 *   swish> exit
 *   $
 * Note that commands like ls are (usually) just programs. There are a few built-in commands, discussed below.
 * In general, though, the shell's job is to launch programs and coordinate their input and output.
 * Important: You do not need to reimplement any binaries that already exist, such as ls.
 * You simply need to launch these programs appropriately and coordinate their execution.
 */

/**
 * Exercise 1. (15 points) Implement simple command parsing in your shell.
 * Upon reading a line, launch the appropriate binary, or detect when the command is a special "built-in" command, such as exit.
 * For now, exit is the only built-in command you need to worry about, but we will add more in the following exercises.
 * Before waiting for input, you should write the shell prompt swish> to the screen.
 * After each command completes, the shell should print another prompt.
 * The shell should print output from commands as output arrives, rather than buffering all output until the command completes.
 * Similarly, if the user is typing input that should go to the running command via stdin, your shell should send these characters as soon as possible, rather than waiting until the user types a newline.
 * You do not need to clear characters from the screen if the user presses backspace.
 * Simply rewrite the command on a new line without the missing character.
 * Note, there is a challenge problem at the end to add backspace support.
 * We will refine the parsing logic in subsequent exercises.
 * Hint: you may want to read the input character by character, as some keystrokes may require action without a newline.
 * Be sure to use the PATH environment variable to search for commands. Be sure you handle the case where a command cannot be found.
 * When you are finished, your shell should be able to execute simple commands like ls and then exit.
 */
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
#include "sys/time.h"
#define MAX_INPUT 1024
#define SHELL_PROMPT "swish>"                               	/* Shell prompt to print before input and after every command */
#define MAX_PATH_LENGTH 255                                    	/* Maximum character length for path name */
#define CH_DIR "cd"                                            	/* Symbolic constant for "cd" command which is used to change directories */
#define PRT_WRK_DIR "pwd"                                       /* Symbolic constant for "pwd" command */
#define EXIT "exit"                                        		/* Symbolic constant for "exit" command */
#define ECHO "echo"                                            	/* Symbolic constant for "echo" command */
#define SET "set"                                            	/* Symbolic constant for "set" command */
#define WOLFIE "wolfie"											/* Symbolic constant for "wolfie" command */
#define ESC_KEY '\033'
#define DELIMITERS " \r\n\t"
#define REDIR_DELIM " <|>\n\t"
#define PATH_ENV "PATH"
#define HOME_ENV "HOME"
#define BACKGROUND "bg" 
#define FOREGROUND "fg"
#define JOBS "jobs"
#define PATH_DELIM ":"
#define ERROR_ARGS "Invalid Arguments\n"
#define ARG_LOC 0
#define PATH_LOC 1
#define OPEN_BRK "["
#define CLOSE_BRK "] "
#define TMP_FILE "~.tmp"
#define _XOPEN_SOURCE 500
#define INVALID_FILE "Invalid File"
#define WAIT_ANY -1 
#define run(msg) fprintf(stderr, "RUNNING: %s\n", msg)
#define end(msg,ex) fprintf(stderr, "ENDED: %s %d\n", msg, ex)
#define time(rel, usr, sys) write(2, "TIME: real=%ds user=%ds system=%ds\n", rel, usr, sys);

/**
 *  You will have to parse the command line and then use fork(2), clone(2), and/or and exec(2)
 *  (or flavors of exec, such as exece, execle, etc.). Programs you run should output to stdout and stderr (errors);
 *  programs you run should take input from stdin. You will have to study the wait(2) system call and its variants,
 *  so your shell can return the proper status codes. Don't spend time writing a full parser in yacc/lex: use plain
 *  str* functions to do your work, such as strtok(3). You may use any system call (section 2 of the man pages) or
 *  library call (section 3 of the man pages) for this assignment, other than system(3).
 *  Hint: Note that, by convention, the name of the binary is the first argument to a program.
 *  Carefully check in the manual of the exec() variant you are using whether you should put the binary name in the
 *  argument list or not.
 *  In general, your selection of libraries is unrestricted, with one important exception:
 *  you should avoid the use of system(), which is really just a wrapper for another shell.
 *  Speaking more broadly, it is not acceptable to simply write a wrapper for another shell---you should implement
 *  your own shell for this assignment.
 */
static int arg_cnt = 0;
static int tflag = 0;
static int dflag = 0;
// static struct timeval begin;
// static struct timeval end;
//static double diff;
extern char **environ;
static history *hist;
static char cmd[MAX_INPUT]; /* line entered from STDIN with a limit on the maximum length of input*/
static char cmd_line[MAX_INPUT]; /* copy of cmd entered by user since strtok null terminates original*/
char *token_array[MAX_INPUT];

int built_in(command * cmd, int ac);

void change_directory(char *path);

int check_key_press(char **c, int position);
void print_time();
void execute_cmd(command *cmd, int a);

char* get_cur_wrk_dir(void);

void get_input(void);

int input_file(char *argv[], command * cmd, char ** tok_arr);

void initialize_cmd(command *cmd, char **token_array);

int isalnumvar(char *str);

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

int main(int argc, char *argv[], char **envp) {
    prompt[0] = '\0'; /* Prompt initially empty, needs to be initialized with correct Shell Prompt */
    int arg_cnt = 0;
    command *cur_cmd = (command *) malloc(sizeof (command));

    char **tokarrp = malloc(sizeof (token_array));
    tokarrp = token_array;

    bool finished = false; /* boolean flag to run SWISH until user chooses to exit */
if(argc > 1){
		  if (!strcmp(*(argv + 1), "-d"))
        dflag = 1;
    if (!strcmp(*(argv + 1), "-t"))
        tflag = 1;

}
    if (argc > 1 && (dflag==0) && (tflag==0))
        return input_file(argv, cur_cmd, tokarrp);

    while (!finished) {
        memset(cmd, '\0', strlen(cmd));
        memset(cmd_line, '\0', strlen(cmd_line));

        print_prompt(prompt);
        get_input();
        arg_cnt = tokenize_input(cmd, &tokarrp, sizeof (cmd), DELIMITERS);
        if (arg_cnt < 1) { /* Checks to see that tokenized command isn't empty (i.e 0) */
            continue;
        }
        initialize_cmd(cur_cmd, token_array);
        execute_cmd(cur_cmd, arg_cnt);
    }
    memset(cmd, '\0', strlen(cmd));
    return 0;
}

// void print_time() {
// struct rusage
// }

/**
 * Check to see if arrow keys or control functions were pressed
 * TODO Flush the buffer otherwise characters get lost
 * @param c
 * @return
 */
int check_key_press(char **c, int position) {
    int rv = 0;
    switch (**c) {
        case '\033':
            getchar();
            switch (getchar()) {
                case 'A':
                    puts("Up arrow pressed\n");
                    break;
                case 'B':
                    puts("Down arrow pressed\n");
                    break;
                case 'C':
                    puts("Right arrow pressed\n");
                    break;
                case 'D':
                    puts("Left arrow pressed\n");
                    break;
                default:
                    break;
            }
            break;
        case '\004': /* END OF TRANSMISSION character */
            break;
    }
    return rv;
}

void left_shift_cmd(int shift_amt) {
    char *cursor = cmd;
    char *shifted_cursor = cmd + shift_amt;
    while ((*cursor++ = *shifted_cursor++))
        ;
    *cursor = '\0';
}

void print_prompt(char *prompt) {
    /* The prompt may change at every user input so best to rewrite it every time*/
    memset(prompt, '\0', strlen(prompt));
    mystrcat(prompt, OPEN_BRK);
    mystrcat(prompt, get_cur_wrk_dir());
    mystrcat(prompt, CLOSE_BRK);
    mystrcat(prompt, SHELL_PROMPT);
    write(STDOUT, prompt, strlen(prompt));
}

void get_input() {
    // read and parse the input
    char *cursor;
    unsigned short count;
    char last_char;
    int rv;

    for (rv = 1, count = 0, cursor = cmd, last_char = 1;
            rv && (++count < (MAX_INPUT - 1))
            && !(last_char == '\n' || last_char == '\0'); cursor++) {
        rv = read(STDIN, cursor, 1);
        check_key_press(&cursor, count);
        last_char = *cursor;
    }
    *cursor = '\0';
    strcpy(cmd_line, cmd);
}

/**
 *
 * @param input_str
 * @param token_array
 * @return The total number of elements stored in token_array
 */
int tokenize_input(char *input_str, char ***tok_arr, int n_tok_arr, char *delimiters) {
    if (*input_str == '\n')
        return -1;

    const char ch = '#';
    char * ret;

    ret = strrchr(cmd, ch);
    if (ret != NULL)
        *ret = '\0';

    //This splits the stdin and puts them in tokenArray!
    int i = 0;
    for (*(*tok_arr + i) = strtok(input_str, delimiters);
            i < n_tok_arr && *(*tok_arr + i) != NULL;
            i++, *(*tok_arr + i) = strtok(NULL, delimiters))
        ;
    return i;
}

void initialize_cmd(command *u_cmd, char **tok_arr) {
    u_cmd->cmd_line = malloc(sizeof (cmd_line));
    u_cmd->cmd_line = cmd_line;
    u_cmd->command = malloc(sizeof (tok_arr[0]));
    u_cmd->command = tok_arr[0];
    u_cmd->args = malloc(sizeof ((tok_arr)));
    u_cmd->args = tok_arr;

    /* TODO make a separate function to initialize and add to history (can do it only if command valid or for every command */
    if (hist == NULL) {
        hist = malloc(sizeof (history));
    }
    hist->prev_cmd = malloc(sizeof (*u_cmd));
    hist->prev_cmd = hist->cur_cmd;
    hist->cur_cmd = malloc(sizeof (*u_cmd));
    hist->cur_cmd = u_cmd;
}

int is_redir_cmd(command *cmd, int ac) {
    int rv = 0, i;
    for (i = 0; i < ac && rv != 1; i++) {
        rv = (is_redir_symbol(**(cmd->args + i))) ? 1 : 0;
    }

    return rv;
}

int is_redir_symbol(char c) {
    return c == REDIR_TO_PROG || c == STDOUT_TO_FILE || c == FILE_TO_STDOUT;
}

void exec_redir(command *cur_cmd, int ac) {
    int i, arg_cnt;
    int fd[2];
    int save_in, save_out, output_fd = -1, input_fd = -1;
    char *cur_tok, *next_tok;
    char cur_tok_c;
    char *orig_cmd_line = malloc(strlen(cur_cmd->cmd_line));
    strcpy(orig_cmd_line, cur_cmd->cmd_line);
    command *new_cmd = malloc(sizeof (command));
    char **tok_arr = malloc(sizeof (token_array));
    for (i = 0; i < ac; i++) {
        cur_tok = *(token_array + i);
        cur_tok_c = cur_tok[0];
        if (is_redir_symbol(cur_tok_c)) {

            next_tok = malloc(strlen(*(token_array + i + 1)));
            next_tok = *(token_array + i + 1);
            switch (cur_tok_c) {
                case STDOUT_TO_FILE: /* '<' character which directs output from a program into a file */
                    save_out = dup(STDOUT_FILENO); /* duplicate stdout file # and store in save_out*/
                    char *file = next_tok;
                    arg_cnt = tokenize_input(cur_cmd->cmd_line, &tok_arr, i - 1, REDIR_DELIM);
                    initialize_cmd(new_cmd, tok_arr);
                    output_fd = open(file, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                    if (output_fd < 0) {
                        puts("Error in accessing that file. Program will now exit.");
                        break;
                    }

                    if (close(STDOUT_FILENO) < 0) {
                        puts("Error in closing stdout. Program will now exit.\n");
                        break;
                    }

                    if (dup(output_fd) != STDOUT_FILENO) {
                        puts("File descriptor is not equal to stdout. Program will now exit.\n");
                        break;
                    }

                    close(output_fd);
                    execute_cmd(new_cmd, arg_cnt);
                    dup2(save_out, STDOUT_FILENO);
                    close(save_out);
                    strcpy(cur_cmd->cmd_line, orig_cmd_line);
                    break;
                case FILE_TO_STDOUT:
                    save_in = dup(STDIN_FILENO); /* Save the file number for stdout and then move it*/
                    file = next_tok;
                    arg_cnt = tokenize_input(cur_cmd->cmd_line, &tok_arr, i - 1, REDIR_DELIM);
                    initialize_cmd(new_cmd, tok_arr);
                    input_fd = open(file, O_RDONLY, S_IRUSR | S_IWUSR);

                    if (input_fd < 0) {
                        puts("Error in accessing that file. Program will now exit.\n");
                        break;
                    }

                    if (close(STDIN_FILENO) < 0) {
                        puts("Error in closing stdout. Program will now exit.\n");
                        break;
                    }

                    if (dup(input_fd) != STDIN_FILENO) {
                        puts("File descriptor is not equal to standard output. Program will now exit.\n");
                        break;
                    }

                    close(input_fd);
                    execute_cmd(new_cmd, arg_cnt);
                    dup2(save_in, fileno(stdin));
                    close(save_in);
                    strcpy(cur_cmd->cmd_line, orig_cmd_line);
                    break;
                case REDIR_TO_PROG:
                    arg_cnt = tokenize_input(cur_cmd->cmd_line, &tok_arr, i - 1, REDIR_DELIM);
                    initialize_cmd(new_cmd, tok_arr);
                    pipe(fd);
                    execute_cmd(new_cmd, arg_cnt);
                    break;
            }
        }
    }
}

int input_file(char* argv[], command *cur_cmd, char ** tok_arr) {
    
		FILE *fp;
    fp = fopen(*(argv + 1), "r");
   
	 if (fp != NULL) {
        while (fgets(cmd, MAX_INPUT, fp) != NULL) {
            arg_cnt = tokenize_input(cmd, &tok_arr, MAX_INPUT, DELIMITERS);
            if (arg_cnt < 1) {
                continue;
            } /* Checks to see that tokenized command isn't empty (i.e 0) */
            initialize_cmd(cur_cmd, token_array);
            execute_cmd(cur_cmd, arg_cnt);
        }
        fclose(fp);
        return 0;
    }
    write(2, INVALID_FILE, strlen(INVALID_FILE));
    return 1;
}

void print_tokens(char **tok_arr) {
    int i;
    for (i = 0; *(tok_arr + i) != NULL; i++) {
        printf("token:%s\n", *(tok_arr + i));
    }
}

void execute_cmd(command *cmd, int ac) { /* ac is arg count */
    int status = 0;
    pid_t wpid;
    struct stat file_stat;
    char *cmd_val = cmd->command;
    char val;
    char * ptr;
    if (dflag) {
        run(cmd->command); /* Debug Message,Runs when specified as -DDEBUG*/
    }
    if (is_redir_cmd(cmd, ac)) {            
        exec_redir(cmd, ac);
    } else if (!built_in(cmd, ac)) {
        if (!strcmp(cmd_val, ECHO)) { /*Did it call Echo? */
            valid_var(cmd->args, ac); /*Method that validates it*/
        }
        
        cmd->pid = fork();
        if (cmd->pid == -1) { /* Error occurred*/
            exit(1);
        } else if (cmd->pid == 0) { /* Inside child process */
            if (file_exists(&cmd->command, file_stat)) {
                printf("executing command\n");
                execvp(cmd->command, cmd->args); /* if execvp returns, it must have failed */
                printf("%s: command not found\n", cmd->command);
                exit(1);
            } else { /* Throw an error */
                puts("File not found\n");
                exit(1);
            }
        } else {
            do {
                wpid = waitpid(cmd->pid, &status, WCONTINUED | WUNTRACED);
                val = WEXITSTATUS(status) + '0'; /*Setting up the '$?' variable */
                ptr = &val;
                setenv("?", ptr, 1);
            } while (wpid > 0);
            if (dflag)
                end(cmd->command, WEXITSTATUS(status));
        }
        /**
         * tokenize the input and then parse it, deal with the multiple pipes using so method and then use the < > to deal with that specific gesture
         * you can use an if statement for that
         */
    }
}

void valid_var(char ** args, int a) { /* Method to Find Arguments That Are Environ var */
    char *beg;
    //char *end;
    int i = 1;
    for (i = 1; i < a; i++) {
        beg = memchr(*(args + i), '$', strlen(*(args + i)));

        if (beg == NULL)
            continue;
        else if (isalnumvar(beg)) {
            *(args + i) = getenv(++beg);
        }
    }
}

int built_in(command * cmd, int ac) {
    char *cmd_val = cmd->command;
    if (!strcmp(cmd_val, EXIT)) {
        exit(1);
    } else if (!strcmp(cmd_val, CH_DIR)) {
        change_directory(cmd->args[PATH_LOC]);
        return 1;
    } else if (!strcmp(cmd_val, PRT_WRK_DIR)) {
        print_working_dir();
        return 1;
    } else if (!strcmp(cmd_val, SET)) { /*Is this setting a new variable */
        set_variable(cmd->args, ac);
        return 1;
    } else if (!strcmp(cmd_val, WOLFIE)) {
        wolfie();
        return 1;
    } else if (!strcmp(cmd_val, BACKGROUND)) {
        return 1;
    } else if (!strcmp(cmd_val, FOREGROUND)) {
        return 1;
    } else if (!strcmp(cmd_val, JOBS)) {
        //print_jobs();
        return 1;
    }

    return 0;
}

int isalnumvar(char * str) { /*Method to check to see if string is all alphanumeric */
    int i = 1;

    if (strlen(str) == 2 && *(str + 1) == '?')
        return 1;

    for (i = 1; i < strlen(str); i++) {
        if (isalnum(*(str + i)))
            continue;
        else {
            return 0;
        }
    }
    return 1;
}

int set_variable(char ** args, int count) {
    if (count == 1 || count == 3 || count > 4) {
        return 1;
    }

    if (count == 4) {
        if (**(args + 2) == '=') {
            setenv(*(args + 1), *(args + 3), 1);
            return 0;
        } else {
            return 1;
        }
    }

    char * var = malloc(sizeof (char) * (strlen(*(args + 1))));
    char *ret;
    char *val = malloc(sizeof (char) * strlen(*(args + 1)));
    ;
    char eq = '=';

    if (count == 2) {
        ret = memchr(*(args + 1), eq, strlen(*(args + 1)));
        memcpy(var, *(args + 1), ret - *(args + 1));
        ret++;
        memcpy(val, ret, strlen(ret));
        setenv(var, val, 1);
    }

    free(var);
    free(val);
    return 0;
}

void change_directory(char *path) {
    if (!path) { /* If path value is empty, go back to home directory */
        chdir(getenv(HOME_ENV)); /* chdir returns 0 on success, -1 on error */
    } else if (chdir(path) != 0) { /* If couldn't successfully change directories */
        printf("Failed to find %s\n", path); /* Throw error */
    }
}

char* get_cur_wrk_dir() {
    return getcwd(NULL, MAX_PATH_LENGTH);
}

void print_working_dir() {
    char *dir_path = get_cur_wrk_dir();
    write(STDOUT, dir_path, strlen(dir_path));
    write(STDOUT, "\n", 1);
}

int file_exists(char **file_path, struct stat file_stat) {
    char *orig_file_path;
    char *concat_file_path;
    orig_file_path = malloc(strlen(*file_path) + 2); /* 1 for '\' and one for '\0' */
    if (**file_path != '/') {
        mystrcat(orig_file_path, "/");
    }

    mystrcat(orig_file_path, *file_path);
    int ret_val = 0;
    char *cur_path;
    char *all_paths = getenv(PATH_ENV);
    cur_path = strtok(all_paths, PATH_DELIM);

    while (cur_path != NULL) {
        concat_file_path = malloc(strlen(orig_file_path) + strlen(cur_path) + 1);
        mystrcat(concat_file_path, cur_path);
        mystrcat(concat_file_path, orig_file_path);
        if ((ret_val = !stat(concat_file_path, &file_stat))) { //Checks if file exists, and value of successful stat is set to ret_val
            *file_path = concat_file_path;
            break;
        }
        cur_path = strtok(NULL, PATH_DELIM);
    }
    return ret_val;
}

