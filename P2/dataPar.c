#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
float *data;

void *findLine(void *len){
	int *interval=(int *)len;
	int start=interval[0],end=interval[1];
	float intercept,slope,dif,intercept_final=0,slope_final=0;
	float SAR=0, SAR_final=(float)INT_MAX;
	for(int i=start;i<end;i++){
		for(int j=i+1;j<=end;j++){
			slope=(data[j]-data[i])/(j-i);
			intercept=data[j]-slope*(j+1);
			for(int k=start;k<=end;k++){
				dif=data[k]-slope*(k+1)-intercept;
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
	printf("The intercept, slope and SAR of the best L1 line of %d data sets are respectively %.2lf, %.2lf and  %.2lf\n",end-start+1,intercept_final,slope_final,SAR_final);
	pthread_exit(0);
}

int main(int argc,char *argv[]){
	if(argc<=1){
		data=(float *)malloc(sizeof(float)*18);
		float d[18]={87.6,88.9,90.4,91.3,92.9,95.4,97.8,100,102.8,104.7,107,109.1,111.5,114.1,114.4,116.5,119.9,121.7};
		for(int i=0;i<18;i++){
			data[i]=d[i];
		}
		pthread_t tid[4];

		int arg[4][2];
		for(int i=0;i<4;i++){
			tid[i]=i;
			arg[i][0]=0;
			arg[i][1]=5+4*i;
			pthread_create(&tid[i],NULL,&findLine,arg[i]);
		}
		pthread_join(tid[0],NULL);
		pthread_join(tid[1],NULL);
		pthread_join(tid[2],NULL);
		pthread_join(tid[3],NULL);
	}
	else{
		data=(float *)malloc(sizeof(float)*3652);
		FILE *stream=fopen(argv[1],"r");
		char line[30];
		fgets(line,30,stream);
		int i=0;
		while(fgets(line,30,stream)){
			char *tok=NULL;
			tok=strtok(line,"\n");
			strtok(tok,",");
			data[i++]=atoi(strtok(NULL,","));
		}
		pthread_t tid[2]={0,1};

		int arg[2][2];
		arg[0][0]=365;
		arg[0][1]=729;
		arg[1][0]=3651;
		arg[1][1]=3651;
		pthread_create(&tid[0],NULL,&findLine,arg[0]);
		pthread_create(&tid[1],NULL,&findLine,arg[1]);
		pthread_join(tid[0],NULL);
		pthread_join(tid[1],NULL);
		fclose(stream);
	}
	free(data);
	exit(1);
}