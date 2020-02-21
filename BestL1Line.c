#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>

float t[18]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
float d[18]={87.6,88.9,90.4,91.3,92.9,95.4,97.8,100,102.8,104.7,107,109.1,111.5,114.1,114.4,116.5,119.9,121.7};

void findLine(void *length){
	float intercept,slope;
	float intercept_final,slope_final;
	float SAR=0, SAR_final=float(INT_MAX);
	for(int i=0;i<*length;i++){
		for(int j=i+1;j<*length;j++){
			slope=(d[j]-d[i])/(t[j]-t[i]);
			intercept=d[j]-slope*t[j];
			for(int k=0;k<*length;k++){
				SAR+=abs(d[k]-slope*t[k]-intercept);
			}
			if(SAR<SAR_final){
				intercept_final=intercept;
				slope_final=slope;
				SAR_final=SAR;
			}
		}
	}
	printf("The intercept and slope of the best L1 line of %d data sets are respectively %.2f and %.2f",*length,intercept_final,slope_final);
}

int main(void){
	p_thread tid[4];
	pthread_attr_t attr[4];
	for(int i=0;i<4;i++){
		pthread_attr_init(&attr[i]);
		int *arg=6+4*i;
		pthread_create(&tid[i],&attr[i],findLine;arg);
	}

}
