#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>
float *data;
double slope;
double intercept;
int start;
int end;



void *seg_SAR(void *args){
	double *arg=(double *)args;	
	int id=(int)*arg;
	int seg_start=start+id*(end-start+1)/4;
	int seg_end=seg_start+(end-start+1)/4-1;
	if(id==3){
		seg_end=end;
	}
	double SAR=0,dif=0;
	for(int k=seg_start;k<=seg_end;k++){
		dif=data[k]-slope*(k+1)-intercept;
		if(dif<0){
			dif*=-1;
		}
		SAR+=dif;
	}
	*arg=SAR;

	pthread_exit(NULL);
}

void findLine(){
	double intercept_final=0,slope_final=0;
	double SAR=0, SAR_final=(double)INT_MAX;
	for(int i=start;i<end;i++){
		for(int j=i+1;j<=end;j++){
			slope=(data[j]-data[i])/(j-i);
			intercept=data[j]-slope*(j+1);
			int numOfThread=4;
		
			pthread_t tid[numOfThread];
			double arg[numOfThread];
			for(int i=0;i<numOfThread;i++){
				tid[i]=i;
				arg[i]=i;	
				pthread_create(&tid[i],NULL,&seg_SAR,&arg[i]);
			}
			for(int i=0;i<numOfThread;i++){
				pthread_join(tid[i],NULL);
			}
			for(int i=0;i<numOfThread;i++){
				SAR+=arg[i];
			}
		
			if(SAR<SAR_final){
				intercept_final=intercept;
				slope_final=slope;
				SAR_final=SAR;
			}
			SAR=0;
		}
	}
	printf("The intercept, slope and SAR of the best L1 line of %d data sets are respectively %.4lf, %.4lf and  %.4f\n",end-start+1,intercept_final,slope_final,SAR_final);
	
}

int main(int argc,char *argv[]){
	struct timeval s;
	gettimeofday(&s,NULL);
	if(argc<=1){
		data=(float *)malloc(sizeof(float)*18);
		float d[18]={87.6,88.9,90.4,91.3,92.9,95.4,97.8,100,102.8,104.7,107,109.1,111.5,114.1,114.4,116.5,119.9,121.7};
		for(int i=0;i<18;i++){
			data[i]=d[i];
		}

		for(int i=0;i<4;i++){

			start=0;
			end=5+4*i;
			findLine();
		}
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

		start=365;
		end=729;
		findLine();
		start=0;
		end=3651;
		//findLine();
		fclose(stream);
	}
	struct timeval e;
	gettimeofday(&e,NULL);
	printf("the runtime of the program is %li.\n",(e.tv_sec-s.tv_sec)*1000000+e.tv_usec-s.tv_usec);
	free(data);
	exit(1);
}
