#include<stdio.h> 
#include<unistd.h> 
#include<string.h>
#define MSGSIZE 16 
char* msg1 = "hello, world #1"; 
int main() 
{ 
    int i; 
      while(1)
{
}
    printf("I am EXEC.c called by execvp() "); 
    //write(1,msg1,MSGSIZE);
    printf("\n"); 
      
    return 0; 
} 
