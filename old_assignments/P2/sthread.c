#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>

float *data;

void findLine(void *len){
	int *interval=(int *)len;
	int start=interval[0],end=interval[1];
	double intercept,slope,dif,intercept_final=0,slope_final=0;
	double SAR=0, SAR_final=(double)INT_MAX;
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
	printf("The intercept, slope and SAR of the best L1 line of %d data sets are respectively %.4lf, %.4lf and  %.4lf\n",end-start+1,intercept_final,slope_final,SAR_final);
}

int main(int argc,char *argv[]){
	struct timeval start;
	gettimeofday(&start,NULL);
	if(argc<=1){
		data=(float *)malloc(sizeof(float)*18);
		float d[18]={87.6,88.9,90.4,91.3,92.9,95.4,97.8,100,102.8,104.7,107,109.1,111.5,114.1,114.4,116.5,119.9,121.7};
		for(int i=0;i<18;i++){
			data[i]=d[i];
		}
		int arg[4][2];
		for(int i=0;i<4;i++){
		
			arg[i][0]=0;
			arg[i][1]=5+4*i;
			findLine(arg[i]);
		}
		
	}
	else{
		data=(float *)malloc(sizeof(float)*3652);
		FILE *stream=fopen(argv[1],"r");
		if(stream==NULL){
			printf("cannot open the file\n");
			free(data);
			exit(0);
		}
		char line[30];
		fgets(line,30,stream);
		int i=0;
		while(fgets(line,30,stream)){
			char *tok=NULL;
			tok=strtok(line,"\n");
			strtok(tok,",");
			data[i++]=atoi(strtok(NULL,","));
		}
		

		int arg[2][2];
		arg[0][0]=365;
		arg[0][1]=729;
		arg[1][0]=0;
		arg[1][1]=3651;
		findLine(arg[0]);
		findLine(arg[1]);
	
		fclose(stream);
	}
	free(data);
	struct timeval end;
	gettimeofday(&end,NULL);
	printf("the program spends %li microseconds.\n",(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec));
	exit(1);
}
