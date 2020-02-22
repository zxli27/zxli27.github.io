#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>

float t[18]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
float d[18]={87.6,88.9,90.4,91.3,92.9,95.4,97.8,100,102.8,104.7,107,109.1,111.5,114.1,114.4,116.5,119.9,121.7};

void *findLine(void *len){
	int length=*((int *)len);
	double intercept,slope,dif,intercept_final=0,slope_final=0;
	double SAR=0, SAR_final=(double)INT_MAX;
	for(int i=0;i<length-1;i++){
		for(int j=i+1;j<length;j++){
			slope=(d[j]-d[i])/(t[j]-t[i]);
			intercept=d[j]-slope*t[j];
			for(int k=0;k<length;k++){
				dif=d[k]-slope*t[k]-intercept;
				if(dif<0){
					dif*=-1;
				}
				SAR+=dif;
			}
		
			if(SAR<SAR_final){
				intercept_final=intercept;
				slope_final=slope;
				SAR_final=SAR;
			}
			SAR=0;
		}
	}
	printf("The intercept, slope and SAR of the best L1 line of %d data sets are respectively %.2lf, %.2lf and  %.2lf\n",length,intercept_final,slope_final,SAR_final);
	pthread_exit(0);
}

int main(void){
	pthread_t tid[4];
	pthread_attr_t attr[4];
	int arg[4];
	//pthread_attr_init(&attr);
	//int arg=6;
	//pthread_create(&tid,&attr,&findLine,&arg);
	//pthread_join(tid,NULL);
	for(int i=0;i<4;i++){
		tid[i]=i;
		pthread_attr_init(&attr[i]);
		arg[i]=6+4*i;
		pthread_create(&tid[i],NULL,&findLine,&arg[i]);
	}
	pthread_join(tid[0],NULL);
	pthread_join(tid[1],NULL);
	pthread_join(tid[2],NULL);
	pthread_join(tid[3],NULL);
	exit(1);
}
