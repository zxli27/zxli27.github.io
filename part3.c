#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

//Phil *phils;
sem_t *mutexs;
int number;

typedef struct Philosopher{
	int one;
	int other;
} Phil;

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
	while(true){
		sem_wait(&mutexs[smaller]);
		sem_wait(&mutexs[larger]);
		printf("philosoper %d is eating with fork %d and fork %d\n",pnum,left,right);
		sleep(3);
		sem_post(&mutexs[larger]);
		sem_post(&mutexs[smaller]);
		printf("philosopher %d is thinking\n",pnum);
	}	
	pthread_exit(0);
}

vint main(int argc,char *argv[]){
	number=*argv[1];
	mutexs=(sem_t *)malloc(sizeof(sem_t)*number);
	//phils=(Phil *)malloc(sizeof(Phil)*number);
	for(int i=0;i<number;i++){
		sem_init(&mutexs[i],0,1);
		//phils[i].one=-1;
		//phils[i].other=-1;
	}
	pthread_t tid[number];
	pthread_attr_t attr[number];
	int arg[number];
	for(int i=0;i<number;i++){
		tid[i]=i;
		pthread_attr_init(&attr[i]);
		arg[i]=i;
		pthread_create(&tid[i],&attr[i],&process,&arg[i]);
	}
	for(int i=0;i<number;i++){
		pthread_join(tid[i],NULL);
	} 
	for(int i=0;i<number;i++){
		sem_destroy(&mutexs[i]);
	}
	//free(phils);

}