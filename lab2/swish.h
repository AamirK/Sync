/*
 * File:   swish.h
 * Author: Wasif
 *
 * Created on March 5, 2015, 9:10 AM
 */

#ifndef SWISH_H
#define	SWISH_H

#include <sys/types.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

/**
* command struct will hold the input line entered into SWISH
* */

typedef struct command {
  pid_t pid;
  char *cmd_line;
  char *command;
  char **args;
}command;

typedef struct cmd_history {
  command *cur_cmd;
  command *prev_cmd;
}history;

char *mystrcat (char *dest, char *src) {
  while (*dest) dest++;
  while ( (*dest++ = *src++) );
  *dest = '\0';
  return --dest;
}

#endif	/* SWISH_H */

