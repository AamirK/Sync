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

#define MAX_INPUT 1024
#define SHELL_PROMPT "swish>"                               	/* Shell prompt to print before input and after every command */
#define MAX_PATH_LENGTH 255                                    	/* Maximum character length for path name */
#define CH_DIR "cd"                                            	/* Symbolic constant for "cd" command which is used to change directories */
#define PWD "pwd"                                            	/* Symbolic constant for "pwd" command */
#define EXIT "exit"                                        		/* Symbolic constant for "exit" command */
#define ECHO "echo"                                            	/* Symbolic constant for "echo" command */
#define SET "set"                                            	/* Symbolic constant for "set" command */
#define ESC_KEY '\033'
#define DELIMITERS " \n\t"
#define PATH_ENV "PATH"
#define PATH_DELIM ":"
#define ERROR_ARGS "Invalid Arguments\n"
#ifdef DEBUG
#define run(msg) fprintf(stderr, "RUNNING: %s\n", msg)
#define end(msg,ex) fprintf(stderr, "ENDED: %s %d\n", msg, ex)
#else
#define run(msg)
#define end(msg, ex)
#endif

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
extern char **environ;

history *hist;
char cmd[MAX_INPUT];
/* line entered from STDIN with a limit on the maximum length of input*/
char *token_array[MAX_INPUT];

int isalnumvar(char *str);
void print_prompt(char *prompt);
void get_input(void);
int tokenize_input(char *input_str, char **token_array);
int check_key_press(char c);
void initialize_cmd(command *cmd);
void execute_cmd(command *cmd, int a);
int set_variable(char ** var_value, int a);
void valid_var(char **args_value, int ar);
void change_directory(void);

int file_exists(char **file_path, struct stat a);

/* TODO program fails when there's no input at all */
int main(int argc, char *argv[]) {
  char prompt[MAX_PATH_LENGTH]; /* prompt to appear at the beginning of every line in SWISH */
  prompt[0] = '\0'; /* Prompt initially empty, needs to be initialized with correct Shell Prompt */
  int arg_cnt = 0;
  command *cur_cmd = (command *) malloc(sizeof(command));

  bool finished = false; /* boolean flag to run SWISH until user chooses to exit *//*TODO finish flag */

  while (!finished) {
	print_prompt(prompt);
	get_input();
	arg_cnt = tokenize_input(cmd, token_array);
	if (arg_cnt > 0) { /* Checks to see that tokenized command isn't empty (i.e 0) */
	  initialize_cmd(cur_cmd); /* TODO flag for successful tokenization */
	} else {
	  continue;
	}

	execute_cmd(cur_cmd, arg_cnt);
  }
  return 0;
}

/**
 * Check to see if arrow keys or control functions were pressed
 * @param c
 * @return
 */
int check_key_press(char c) {
  switch (c) {
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
		  return 0;
	  }
	case '\004':
	  exit(0);
  }
  return 0;
}

void print_prompt(char *prompt) {
  if (strlen(prompt) < 1) {
	mystrcat(prompt, SHELL_PROMPT);
  }
  write(STDOUT, prompt, NELEMS(prompt));
}

void get_input() {
  // read and parse the input
  char *cursor;
  unsigned short count;
  char last_char;
  int rv;

  for (rv = 1, count = 0, cursor = cmd, last_char = 1;
	  rv && (++count < (MAX_INPUT - 1)) && (last_char != '\n'); cursor++) {
	rv = read(STDIN, cursor, 1);
	//check_key_press(*cursor);
	last_char = *cursor;
  }
  *cursor = '\0';
}

/**
 *
 * @param input_str
 * @param token_array
 * @return The total number of elements stored in token_array
 */
int tokenize_input(char *input_str, char **token_array) {
  if (*input_str == '\n') {
	return -1;
  }

  //This splits the stdin and puts them in tokenArray!
  int i = 0;
  for (token_array[i] = strtok(cmd, DELIMITERS);
	  i < MAX_INPUT && token_array[i] != NULL;
	  i++, token_array[i] = strtok(NULL, DELIMITERS))
	;

  return i;
}

/*TODO make token_array a parameter to make this more extensible */
void initialize_cmd(command *cmd) {
  cmd->command = malloc(sizeof(token_array[0]));
  cmd->command = token_array[0];
  cmd->args = malloc(sizeof((token_array)));
  cmd->args = token_array;

  /* TODO make a separate function to initialize and add to history (can do it only if command valid or for every command */
  if (hist == NULL) {
	hist = malloc(sizeof(history));
  }
  hist->cur_cmd = (command *) malloc(sizeof(*cmd));
  hist->cur_cmd = cmd;
}

void execute_cmd(command *cmd, int ac) { /* TODO check to make sure command isn't empty */
  int status = 0;
  pid_t wpid;
  struct stat file_stat;
  char *cmd_val = cmd->command;

  run(cmd->command); /* Debug Message,Runs when specified as -DDEBUG*/
  if (!strcmp(cmd_val, EXIT)) {
	exit(0);
  } else if (!strcmp(cmd_val, CH_DIR)) {
	change_directory();
  } else if (!strcmp(cmd_val, SET)) { /*Is this setting a new variable */
	set_variable(cmd->args, ac);
  } else {
	if (!strcmp(cmd_val, ECHO)) { /*Did it call Echo? */
	  valid_var(cmd->args, ac); /*Method that validates it*/
	}
	cmd->pid = fork();
	if (cmd->pid == -1) { /* Error occurred*/
	  exit(1);
	} else if (cmd->pid == 0) { /* Inside child process */
	  if (file_exists(&cmd->command, file_stat)) {
		execvp(cmd->command, cmd->args);
	  } else { /* Throw an error */

	  }
	} else {
	  do {
		wpid = waitpid(cmd->pid, &status, WCONTINUED | WUNTRACED);
	  } while (wpid > 0);end(cmd->command, WEXITSTATUS(status)); /* Debug Message,Runs when specified as -DDEBUG*/
	}

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
int isalnumvar(char * str) { /*Method to check to see if string is all alphanumeric */
  int i = 1;
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

  char * var = malloc(sizeof(char) * (strlen(*(args + 1))));
  char *ret;
  char *val = malloc(sizeof(char) * strlen(*(args + 1)));
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
void change_directory() {
}

int file_exists(char **file_path, struct stat file_stat) {
  int ret_val = 0;
  char *all_paths = getenv(PATH_ENV);
  char *cur_path;
  cur_path = strtok(all_paths, PATH_DELIM);
  while (cur_path) {
	if (**file_path != '/') {
	  mystrcat(cur_path, "/");
	}
	mystrcat(cur_path, *file_path);
	if ((ret_val = !stat(cur_path, &file_stat))) { //This means that the file exists, and the value of successful stat is set to ret_val
	  *file_path = cur_path;
	  break;
	}
	cur_path = strtok(NULL, PATH_DELIM);
  }
  return ret_val;
}

