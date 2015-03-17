#ifndef REDIRECTION_H

typedef enum redir_t {
  STDOUT_TO_FILE 	= '>',
  FILE_TO_STDOUT 	= '<',
  REDIR_TO_PROG 	= '|'
}redir_t;


#define REDIRECTION_H
#endif /* REDIRECTION_H */
