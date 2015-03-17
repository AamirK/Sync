#include <string.h>
#include <stdio.h>


int test()
{
   const char str[80] = "This is - www.tutorialspoint.com - website";
   const char s[2] = "-";
   char *token;
   char * bob;
   /* get the first token */
   token = strtok(str, s);
   
   /* walk through other tokens */
   while( token != NULL ) 
   {
      printf( " %s\n", token );
    
      bob = strtok(NULL, s);
	  printf(" %s\n", bob);
   }
   
   return(0);
}
