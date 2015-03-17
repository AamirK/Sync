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

typedef struct  {
  command *cmd;
  bool is_background;
  char *name = cmd->cmd_line;
  int job_number;
} job;