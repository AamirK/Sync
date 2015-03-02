/* CSE 306: Sea Wolves Interactive SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#define MAX_INPUT 1024
//THIS STUFF IS FOR PART 4 
#ifdef DEBUG
	#define run(msg) fprintf(stderr, "RUNNING: %s\n", msg)
	#define end(msg,ex) fprintf(stderr, "ENDED: %s %d\n", msg, ex)
#else
	#define run(msg)
	#define end(msg,ex)
#endif

int 
main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "swish> ";
  char cmd[MAX_INPUT];
  char *EX = "exit";
  char OB[50];
  char CB[3];
  char * tokenArray[512];
  char * token;
  char * cwd = NULL;
  char * wd;	
 strcpy(OB, "["); //THIS is the braccket for part 3
 strcpy(CB, "] "); //ALSO part 3
 cwd = getcwd(cwd, 4096); // This gets the working directory inside CWD
	/*This sets wd to the last directory
	*For example if i had /user/aamir/cse306 
		it would return me just /cse306
	FYI: I'm not sure Porter wants the ENTIRE directory. 
	*/
	wd = strrchr(cwd, '/'); 

	strcat(OB, wd);
	strcat(OB, CB);
	strcat(OB, prompt);
	
	while (!finished) {
    char *cursor;
    char last_char;
    int rv;
    int count;
	int i =0;
	
    rv = write(1, OB, strlen(OB)); // Print the prompt
    if (!rv) { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1; 
		rv && (++count < (MAX_INPUT-1)) && (last_char != '\n'); 
		cursor++) { 
      rv = read(0, cursor, 1);
      last_char = *cursor;	} 
    
	*cursor = '\0';
	//This splits the stdin and puts them in tokenArray!
	for(tokenArray[i] = strtok(cmd," \n"); 
		i<512 && tokenArray[i] !=NULL; 
		i++, tokenArray[i] = strtok(NULL, " \n") ){}
	
	token = strtok(cmd, " \n");
	
	run(token); //This is for part 4 the debugger thing
	if (!rv) { 
       finished = 1;
       break;
     }	 
    // Execute the command, handling built-in commands separately 
	 pid_t child_pid = fork();
	*(token +strlen(token)) = '\0';	 //This takes out the newLine character in the input
	
		while (token !=NULL){
		/*This checks to see if the token matches the EXIT command
		He said we need to account for a special case I think
		*/
		if(!strncmp(EX, token,4)){
				end(token,0); //Part 4 stuff
				exit(0);
		}
		//If the child_process was created, execute the function
		else if (child_pid == 0){
			 execvp(token, tokenArray); 
			 //end(token, 3);
			 exit(3);	}
		else{
			wait(&child_pid);}
		
		token = strtok(NULL, " \n");	}
		end(token,3);
	}
	//end(token,3);
  return 0;
}
