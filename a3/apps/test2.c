#include <stdio.h>
#include <string.h>

#include "../io/File.h"

int main(){
	printf("1. initialize and format the disk.\n");
	initDisk();
	printf("2. build 17 directories under root.\n");
	char dirs[17][7];
	for(int i=0;i<17;i++){
		memcpy(dirs[i],"/dir",4);
		dirs[i][4]=(i/10)+'0';
		dirs[i][5]=(i%10)+'0';
		dirs[i][6]='\0';
		createFile(dirs[i],1);
	}
	printf("3. create a file named file1 in dir00 and write 1500 a's into it.\n");
	createFile("/dir00/file1",0);
	char a[1501];
	for(int i=0;i<1500;i++){a[i]='a';}
	a[1500]='\0';
	writeFile("/dir00/file1",a);
	printf("4. read 1000 bytes from the file: ");
	char ptr[501];
	readFile("/dir00/file1",1000,ptr);
	printf("%s\n",ptr);
	printf("5 delete dir01.\n");
	deleteFile("/dir01");
	printf("create dir17.\n");
	createFile("/dir17",1);


}
