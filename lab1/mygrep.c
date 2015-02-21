
#include "mysyscall.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define HELP_MESSAGE "-c NUM: Stop after NUM matches (or when out of input). The default is to search all input.\n \
-h: Print a help message and exit.\n-r: Search recursively into the children if the search target is a directory.\n \
-v: Reverse the pattern, i.e., print lines that do not match the pattern."
#define ERROR_ARGUMENTS "ERROR: Improper Number of Arguments\n"
#define HYPHEN "-";
#define MAXBUFFER 512
#define CONSOLEBUFF 4096
#define EXIT_FAILURE -2
#define INPUT_ERROR "ERROR: Input File Can't Be Opened\n"
#define READ_ERROR "ERROR: Read File Can't Be Opened\n"
#define OUT_ERROR "ERROR: Out File Can't Be Opened\n"
#define WRT_ERROR "ERROR: File Can't Be Written To\n"
void util_start(void);
int stringLen( char * str);
int stringCmp(const char * a, const char *b);
asm (".global util_start\r\n"
     "  .type util_start,@function\r\n"
     ".global main\r\n"
     "  .type main,@function\r\n");

/* Get the argc and argv values in the right registers */
asm ("util_start:\r\n"
     "movl %esp, %eax\r\n"
     "addl $4, %eax\r\n"
     "pushl %eax\r\n"
     "subl $4, %eax\r\n"
     "pushl (%eax)\r\n"
     "  call main\r\n");
int main(int argc, char **argv) {
int return_code = argc;
int input_fd, output_fd, rd_count,wrt_count,sL,flag;
char bf [CONSOLEBUFF];
char * tmp = bf; 
char * wrd = argv[argc-3]; 
sL = stringLen(wrd);	
char * nnL;
char * match;
	if	((input_fd = MY_OPEN(argv[argc-2], O_RDONLY))< 0){
			MY_WRITE(2, INPUT_ERROR,35);
			 MY_EXIT(EXIT_FAILURE);			
			return EXIT_FAILURE;	}
	if(*argv[argc-1] != '-'){
	if  ((output_fd = MY_OPEN2(argv[argc-1], O_RDWR|O_CREAT,S_IRWXU)) < 0){
			MY_WRITE(2, OUT_ERROR,34);
			MY_EXIT(EXIT_FAILURE);		} 
	}
	else{
		output_fd = 1;
	}
	
	if(*argv[argc-2] == '-'){
		rd_count =	MY_READ(0, bf, CONSOLEBUFF);}
	else{
		rd_count = MY_READ(input_fd, bf, CONSOLEBUFF);
			if(rd_count < 0) {
				MY_WRITE(2, READ_ERROR,34);
				MY_EXIT(EXIT_FAILURE);	}
			nnL = tmp;
			match = tmp;
		while(*tmp != '\0' && *nnL != '\0'){
			
			while(*match != *wrd){
				match++;
			}
			if(*match == *wrd){
				int i = 1;
					while (* (match+i) == *(wrd + i) && i < sL){
						i++;
					}
				if (i == sL){flag=1;}
				else{match++;}
			}
			
			while(*nnL != '\n'){
				nnL++;
			}
			if(match - nnL > 0 && flag ==1){
				nnL++;
				tmp = nnL;
			}
			if( (nnL - match) ==0 && flag == 1){
				nnL++;
				wrt_count =	MY_WRITE(output_fd, tmp, nnL-tmp);
			if(wrt_count < 0) {
				MY_WRITE(2, WRT_ERROR,29);
				MY_EXIT(EXIT_FAILURE);	}
				match++;
				tmp = nnL;
			flag =0;
			}
			if(match - nnL < 0 && flag ==1){
			nnL++;
			wrt_count =	MY_WRITE(output_fd, tmp, nnL-tmp);
			if(wrt_count < 0) {
				MY_WRITE(2, WRT_ERROR,29);
				MY_EXIT(EXIT_FAILURE);	}
			match++;
			tmp = nnL;
			flag =0;
			}
			
			
		}	
			
		}
			
			
			
		
			


  MY_EXIT(return_code);
  return return_code;
}
int stringCmp(const char *str1, const char *str2){

	if ( (str1==NULL) || (str2==NULL) )
		return 0;

	for( ;*str1 == *str2; str1++, str2++)
		if (*str2=='\0')
			return 1;
		return 0;
	}

int stringLen(char *str){
	if(str == NULL)
		return 0;
		size_t a;
		for( a=0;*str!= '\0';str++) {
				a++;}
		return a;
	}
