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
#include <signal.h>
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
#include <sys/resource.h>
#include <string.h>
#include "swish.h"
#include "wolfie.h"

#define MAX_INPUT 1024
#define SHELL_PROMPT "swish>"                               	/* Shell prompt to print before input and after every command */
#define MAX_PATH_LENGTH 255                                    	/* Maximum character length for path name */
#define CH_DIR "cd"                                            	/* Symbolic constant for "cd" command which is used to change directories */
#define PRT_WRK_DIR "pwd"                                       /* Symbolic constant for "pwd" command */
#define EXIT "exit"                                        		/* Symbolic constant for "exit" command */
#define ECHO "echo"                                            	/* Symbolic constant for "echo" command */
#define SET "set"                                            	/* Symbolic constant for "set" command */
#define WOLFIE "wolfie"											/* Symbolic constant for "wolfie" command */
#define BACKGROUND "bg"
#define FOREGROUND "fg"
#define JOBS "jobs"
#define ESC_KEY '\033'
#define DELIMITERS " \r\n\t"
#define REDIR_DELIM " \n\t<|>"
#define PATH_ENV "PATH"
#define HOME_ENV "HOME"
#define PATH_DELIM ":"
#define ERROR_ARGS "Invalid Arguments\n"
#define ARG_LOC 0
#define PATH_LOC 1
#define OPEN_BRK "["
#define CLOSE_BRK "] "
#define INVALID_FILE "Invalid File"
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
static history *hist;
static int arg_cnt =0;
static jobs* job_List; 
char cmd[MAX_INPUT]; /* line entered from STDIN with a limit on the maximum length of input*/

char *token_array[MAX_INPUT];

void change_directory(char *path);

int check_key_press(char c, int position);

void execute_cmd(command *cmd, int a);

char* get_cur_wrk_dir(void);

void get_input(void);

void initialize_cmd(command *cmd);

int isalnumvar(char *str);

void print_prompt(char *prompt);

void print_working_dir(void);

void handler(int sig);

int set_variable(char ** var_value, int a);

int tokenize_input(char **token_array, char *delimiters);

void valid_var(char **args_value, int ar);

char prompt[MAX_PATH_LENGTH]; /* prompt to appear at the beginning of every line in SWISH */

int file_exists(char **file_path, struct stat a);

int built_in(command * cmd, int ac);

int input_file(char *argv[], command * cmd);

int main(int argc, char *argv[], char **envp) {
  prompt[0] = '\0'; /* Prompt initially empty, needs to be initialized with correct Shell Prompt */
  command *cur_cmd = (command *) malloc(sizeof(command));
  bool finished = false; /* boolean flag to run SWISH until user chooses to exit */
  if (argc > 1) return input_file(argv, cur_cmd);
  while (!finished) {
	print_prompt(prompt);
	get_input();
	arg_cnt = tokenize_input(token_array, DELIMITERS);
	if (arg_cnt < 1) { /* Checks to see that tokenized command isn't empty (i.e 0) */
	  continue;
	}
	initialize_cmd(cur_cmd);
	execute_cmd(cur_cmd, arg_cnt);
  }
  return 0;
}

/**
 * Check to see if arrow keys or control functions were pressed
 * TODO Flush the buffer otherwise characters get lost
 * @param c
 * @return
 */
int check_key_press(char c, int position) {
  int rv = 0;
  int count, arg_cnt, cur_char;
  int out, err, save_out, save_err;
  int output_fd = -1;
  command *new_cmd = malloc(sizeof(command));
  char file_path[MAX_PATH_LENGTH];
  char *f_ptr = file_path;
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
		  break;
	  }
	  break;
	case '\004': /* END OF TRANSMISSION character */
	  puts("went here\n");
	  break;
	case STDOUT_TO_FILE: /* '<' char for redirection support */
	  /**
	   * When the < is seen then the first part will be the command to execute or string to add to the file
	   * A new command will be made for this first part (before the '<')
	   * The second part will be the file to write to.
	   * In order to write to the output file, the other output streams need to be saved and closed.
	   */
	  arg_cnt = tokenize_input(token_array, REDIR_DELIM);
	  initialize_cmd(new_cmd);
	  for (count = 0, cur_char = getchar();
		  cur_char != EOF && cur_char != '\n' && count < MAX_PATH_LENGTH;
		  *f_ptr++ = cur_char, cur_char = getchar(), ++count)
		;
	  *f_ptr = '\0';

	  printf("file path is:%s new_cmd is: %s\n", file_path, *(new_cmd->args));

	  output_fd = open(file_path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	  if (output_fd < 0) {
		perror("open(2) file: ");
		printf("%s\n", file_path);
		rv = EXIT_FAILURE;
		break;
	  }

	  if (close(STDOUT_FILENO) < 0) {
		perror("close(2) file: STDOUT_FILENO");
		close(output_fd);
		rv = EXIT_FAILURE;
		break;
	  }

	  //save_out = dup(fileno(stdout));
	  //execute_cmd(new_cmd, arg_cnt);
	  break;
  }
  return 0;
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
	int rw;
  for (rv = 1, count = 0, cursor = cmd, last_char = 1;
	  rv && (++count < (MAX_INPUT - 1)) && (last_char != '\n'); cursor++) {
	rv = read(STDIN, cursor, 1);
	rw = write(STDOUT, cursor, 1);
	check_key_press(*cursor, count);
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
int tokenize_input(char **token_array, char *delimiters) {
 
	const char ch = '#';
	char * ret;
	
	ret = strrchr(cmd, ch);
	if(ret != NULL)
		*ret = '\0';

  //This splits the stI in and puts them in tokenArray!
  int i = 0;
  for (token_array[i] = strtok(cmd, delimiters);
	  i < MAX_INPUT && token_array[i] != NULL;
	  i++, token_array[i] = strtok(NULL, delimiters))
	;
  return i;
}

void initialize_cmd(command *cmd) {
  cmd->command = malloc(sizeof(token_array[0]));
  cmd->command = token_array[0];
  cmd->args = malloc(sizeof((token_array)));
  cmd->args = token_array;

  /* TODO make a separate function to initialize and add to history (can do it only if command valid or for every command */
  if (hist == NULL) {
	hist = malloc(sizeof(history));
  }
  hist->prev_cmd = malloc(sizeof(*cmd));
  hist->prev_cmd = hist->cur_cmd;
  hist->cur_cmd = malloc(sizeof(*cmd));
  hist->cur_cmd = cmd;
}


void launch_cmd(command* cmd, int ac){
	pid_t pid;
	
}

void execute_cmd(command *cmd, int ac) { /* ac is arg count */
  int status = 0;
  pid_t wpid;
	pid_t pid;
	int bflag = 0;
	struct stat file_stat;
	struct sigaction sa;
	sa.sa_handler = handler;
	run(cmd->command); /* Debug Message,Runs when specified as -DDEBUG*/
  
	if(!built_in(cmd, ac)){
		if (!strcmp(cmd->command, ECHO)) { /*Did it call Echo? */
				valid_var(cmd->args, ac);} /*Method that validates it*/
		
		if(*(cmd->args[ac-1]) == '&'){
				cmd->args[ac-1] = '\0';
				bflag ==1;
		}	
		
		pid = fork();
		
		switch(pid) :
			
			case -1: 
							exit(1);
		
		if (pid == -1) { /* Error occurred*/
						exit(1);} 
		else if (pid == 0) { /* Inside child process */
						sigaction(SIGCHLD, &sa, NULL);		
		
		if (file_exists(&cmd->command, file_stat) && bflag){
					execvp(cmd->command, cmd->args); /* if execvp returns, it must have failed */
					puts("Unknown");
					exit(0);
			} 
			else{ /* Throw an error */
					puts("File not found");
					exit(1);
			}
		} 
		else {
			if(bflag){
			
		}
	  
	}
}
return; 
	
}

int built_in(command * cmd, int ac){
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
  }
	else if( !strcmp(cmd_val, BACKGROUND)){
	
		return 1;
	}
	else if(!strcmp(cmd_val, FOREGROUND)){
		
		return 1;
	}
	else if(!strcmp(cmd_val, JOBS)){
		//print_jobs();
		return 1;
	}
	
	return 0;
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
  
	if(strlen(str) ==2 && *(str+1) =='?')
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
void change_directory(char *path) {
  if (!path) { /* If path value is empty, go back to home directory */
	chdir(getenv(HOME_ENV)); /* chdir returns 0 on success, -1 on error */
  } else if (chdir(path) != 0) { /* If couldn't successfully change directories */
	printf("Failed to find %s\n", path); /* Throw error */
  }
}
// void print_jobs(){
	
// }
int input_file(char* argv[], command *cur_cmd){
	FILE *fp;
	fp = fopen(*(argv + 1), "r");
	if (fp != NULL) {
	  while (fgets(cmd, MAX_INPUT, fp) != NULL) {
		arg_cnt = tokenize_input(token_array, DELIMITERS);
		if (arg_cnt < 1) {
		  continue;
		} /* Checks to see that tokenized command isn't empty (i.e 0) */
		initialize_cmd(cur_cmd);
		execute_cmd(cur_cmd, arg_cnt);

	  }
	  fclose(fp);
		return 0;
	}
	write(2, INVALID_FILE, strlen(INVALID_FILE));
	return 1;
}
void handler(int sig){
	pid_t wpid; 
	int status;
	char val; 
	char * ptr; 
	do {
		wpid = waitpid(-1, &status, WCONTINUED | WUNTRACED);
		val = WEXITSTATUS(status) + '0'; /*Setting up the '$?' variable */
		ptr = &val;
		setenv("?", ptr, 1);
		end(cmd->command, WEXITSTATUS(status)); /* Debug Message,Runs when specified as -DDEBUG*/
		} while (wpid > 0);
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
  char *concat_path;
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

