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
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>									/* unistd, sys/stat and sys/types required for stat() system call */
#include <sys/stat.h>
#include <sys/types.h>
#include "swish.h"

#define MAX_INPUT 1024
#define SHELL_PROMPT "swish>"								/* Shell prompt to print before input and after every command */
#define MAX_PATH_LENGTH 255								/* Maximum character length for path name */
#define CH_DIR "cd"									/* Symbolic constant for "cd" command which is used to change directories */
#define PWD "pwd"									/* Symbolic constant for "pwd" command */
#define EXIT "exit"									/* Symbolic constant for "exit" command */
#define ESC_KEY '\033'
#define DELIMITERS " \n\t"
#ifdef DEBUG
#define run(msg) fprintf(stderr, "RUNNING: %s\n", msg)
#define end(msg,ex) fprintf(stderr, "ENDED: %s %d\n", msg, ex)
#else
#define run(msg)
#define end(msg,ex)
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

history *hist;
char cmd[MAX_INPUT];                            /* line entered from STDIN with a limit on the maximum length of input*/
char *token_array[MAX_INPUT];


void print_prompt (char *prompt);
void get_input(void);
int tokenize_input(char *input_str, char **token_array);
int check_arrow_press (char c);
void initialize_cmd(command *cmd);
void execute_cmd (command *cmd);
void change_directory(void);

/* TODO program fails when there's no input at all */
int main(int argc, char *argv[]) {
  char prompt[MAX_PATH_LENGTH];                 /* prompt to appear at the beginning of every line in SWISH */
  prompt[0] = '\0';                             /* Prompt initially empty, needs to be initialized with correct Shell Prompt */

  command *cur_cmd = (command*) malloc(sizeof(command));

  bool finished = false;                        /* boolean flag to run SWISH until user chooses to exit */ /*TODO finish flag */

  while (!finished) {
    print_prompt(prompt);
    get_input();

    if (tokenize_input(cmd, token_array)) {		/* Checks to see that tokenized command isn't empty (i.e 0) */
      initialize_cmd(cur_cmd);					/* TODO flag for successful tokenization */
    }
	execute_cmd(cur_cmd);
  }
  return 0;
}

int check_arrow_press (char c) {
  if (c == '\033') {
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
       rv && (++count < (MAX_INPUT - 1)) && (last_char != '\n');
       cursor++) {
    rv = read(STDIN, cursor, 1);
    check_arrow_press(*cursor);
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
  //This splits the stdin and puts them in tokenArray!
  int i = 0;
  for (token_array[i] = strtok(cmd, DELIMITERS);
       i < MAX_INPUT && token_array[i] != NULL;
       i++, token_array[i] = strtok(NULL, DELIMITERS))
    ;

  return NELEMS(token_array);
}

/*TODO make token_array a parameter to make this more extensible */
void initialize_cmd (command *cmd) {
  cmd->command = malloc(sizeof(token_array[0]));
  cmd->command = token_array[0];
  cmd->args = malloc(sizeof((token_array + 1)));
  cmd->args = token_array + 1;

  /* TODO make a separate function to initialize and add to history (can do it only if command valid or for every command */
  if (hist == NULL) {hist = malloc(sizeof(history));}
  hist->cur_cmd = (command*) malloc(sizeof(*cmd));
  hist->cur_cmd = cmd;
}

void execute_cmd (command *cmd) { /* TODO check to make sure command isn't empty */
  struct stat fileStat;
  char *cmd_val = cmd->command;
  if (!strcmp(cmd_val, EXIT)) {
	exit(0);
  } else if (!strcmp(cmd_val, CH_DIR)) {
	change_directory();
  } else {
	cmd->pid = fork();
	if (cmd->pid == -1) {							/* Error occurred*/
	  exit(1);
	} else if (cmd->pid == 0) {						/* Inside child process */

	  printf("%s\n", getenv("PATH"));
	  printf("%d\n", stat("ls", &fileStat));
	  /*
	   printf("token:%s\n", cmd_val);
	  if ( (stat(cmd_val, &fileStat)) < 0) {
		printf("couldn't find the file with command value:%s", cmd_val);
		exit(1);
	  } else {
		printf("it does exit so do shit\n");
	  }
	   */
	  exit(0);
	} else  {
//	  wait();
	}
  }
}

void change_directory() {}
