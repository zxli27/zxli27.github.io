#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

sem_t *mutexs;
int number;

/*typedef struct Philosopher{
	int one;
	int other;
} Phil;*/

void *process(void *arg){
	int pnum=*((int *)arg);
	int left=(pnum+number-1)%number;
	int right=pnum%(number-1);
	int smaller,larger;
	if(left<right){
		smaller=left;
		larger=right;
	}
	else{
		smaller=right;
		larger=left;
	}
	while(1){
		sem_wait(&mutexs[smaller]);
		sem_wait(&mutexs[larger]);
		printf("philosoper %d is eating with fork %d and fork %d\n",pnum,left,right);
		sleep(3);
		printf("philosopher %d is thinking\n",pnum);
		sem_post(&mutexs[larger]);
		sem_post(&mutexs[smaller]);
		sleep(3);
	}	
	pthread_exit(0);
}

int main(int argc,char *argv[]){
	number=atoi(argv[1]);
	mutexs=(sem_t *)malloc(sizeof(sem_t)*number);
	//phils=(Phil *)malloc(sizeof(Phil)*number);
	for(int i=0;i<number;i++){
		sem_init(&mutexs[i],0,1);
		//phils[i].one=-1;
		//phils[i].other=-1;
	}
	pthread_t tid[number];
	
	int arg[number];
	for(int i=0;i<number;i++){
		tid[i]=i;
	
		arg[i]=i;
		pthread_create(&tid[i],NULL,&process,&arg[i]);
	}
	for(int i=0;i<number;i++){
		pthread_join(tid[i],NULL);
	} 
	for(int i=0;i<number;i++){
		sem_destroy(&mutexs[i]);
	}
	free(mutexs);
	//free(phils);

}
