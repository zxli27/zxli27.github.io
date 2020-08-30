#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>
float *data;
int start;
int end;
double fslope[10];
double fintercept[10];
double fSAR[10];

void *findLine1(void *len){
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

	printf("The intercept, slope and SAR of the best L1 line of %d data sets are respectively %.4lf, %.4lf and  %.4f\n",end-start+1,intercept_final,slope_final,SAR_final);
	pthread_exit(0);
}

void *seg_SAR(void *arg){
	int id=*((int *)arg);
	double dif,slope,intercept,intercept_final=0,slope_final=0;
	double SAR=0, SAR_final=(double)INT_MAX;
	int seg_start=start+id*(end-start+1)/10;
	int seg_end=seg_start+(end-start+1)/10-1;
	if(id==9){
		seg_end=end;
	}
	for(int i=seg_start;i<=seg_end;i++){
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
	fslope[id]=slope_final;
	fintercept[id]=intercept_final;
	fSAR[id]=SAR_final;
	
	pthread_exit(0);
}

void findLine2(){
	double intercept_final=0,slope_final=0;
	double SAR_final=(double)INT_MAX;
		
	pthread_t tid[10];
	int arg[10];
	for(int i=0;i<10;i++){
		tid[i]=i;
		arg[i]=i;	
		pthread_create(&tid[i],NULL,&seg_SAR,&arg[i]);
	}
	for(int i=0;i<10;i++){
		pthread_join(tid[i],NULL);
	}
	for(int i=0;i<10;i++){
		if(fSAR[i]<SAR_final){
			intercept_final=fintercept[i];
			slope_final=fslope[i];
			SAR_final=fSAR[i];
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
		pthread_t tid[4];

		int arg[4][2];
		for(int i=0;i<4;i++){
			tid[i]=i;
			arg[i][0]=0;
			arg[i][1]=5+4*i;
			pthread_create(&tid[i],NULL,&findLine1,arg[i]);
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

		start=365;
		end=729;
		findLine2();
		start=0;
		end=3651;
		findLine2();
		fclose(stream);
	}
	struct timeval e;
	gettimeofday(&e,NULL);
	printf("the runtime is %li\n",(e.tv_sec-s.tv_sec)*1000000+e.tv_usec-s.tv_usec);
	free(data);
	exit(1);
}
