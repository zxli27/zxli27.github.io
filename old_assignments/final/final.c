#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
	pid_t pid=fork();
	if(pid==0){
		printf("I am the child %d\n",getpid());
	}
	else{
		printf("I am %d,the parent of %d\n",getpid(),pid);
		wait(NULL);
	}
	pid=fork();
	if(pid==0){
		printf("I am the child %d\n",getpid());
	}
	else{
		printf("I am %d,the parent of %d\n",getpid(),pid);
		wait(NULL);
	}
	pid=fork();
	if(pid==0){
		printf("I am the child %d\n",getpid());
	}
	else{
		printf("I am %d,the parent of %d\n",getpid(),pid);
		wait(NULL);
	}


	return 0;
}
