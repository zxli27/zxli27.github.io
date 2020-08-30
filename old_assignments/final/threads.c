#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h> 

void *child(void *arg) { 
	printf("I am the child %d\n",(int)pthread_self());
	execlp("/bin/ls","ls",NULL);
	printf("Child Complete\n");
    return NULL; 
} 
int main()
{
    pthread_t tid; 
  	printf("I am the parent %d\n",(int)pthread_self());
    pthread_create(&tid, NULL, child, NULL); 
    pthread_join(tid, NULL); 
    return 0;
}