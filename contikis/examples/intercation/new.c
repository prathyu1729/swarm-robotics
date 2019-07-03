#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include<string.h> 
#include <stdlib.h>
#define MSGSIZE 16 
int main()
{


		
		int fd[2];
	      	if(pipe(fd)<0)
	      	{
		     printf("Error in creating pipes ");  
		     exit(1) ;
	      	}
                int p=fork();
		if(p>0)
		{
		 sleep(1);
                 //printf("in parent");
		 close(fd[1]);
                 //wait(NULL);
		 char inbuf[MSGSIZE]; 
                 bzero(inbuf,MSGSIZE);
        	 read(fd[0], inbuf, MSGSIZE); 
        	 printf("%s received\n",inbuf);		 
		 

		}
		else
		{
		  //printf("in child");
		  close(fd[0]);
		  //dup2(fd[1],fileno(stdout));   
		  char *args[]={"../../tools/sky/serialdump-linux -b115200 /dev/ttyUSB2 ",NULL}; 
			//char *args[]={"./test ",NULL}; 	
        	  execvp(args[0],args); 
		 printf("failed");
		  
		
		}


	return 0;

}
