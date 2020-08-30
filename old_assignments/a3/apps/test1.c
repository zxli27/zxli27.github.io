#include <stdio.h>

#include "../io/File.h"

int main(){
	printf("1. initialize and format the disk.\n");
	initDisk();
	printf("2. create and interact with a file under root directory.\n");
	printf("	create a file named file1\n");
	createFile("/file1",0);
	printf("	write Hello,World! into file1\n");
	writeFile("/file1","Hello,Wolrd!");
	printf("	read 3 bytes from file1: ");
	char ptr[10];
	readFile("/file1",3,ptr);
	printf("%s\n",ptr);
	printf("3. delete file1\n");
	deleteFile("/file1");
	

	printf("4. create a directory.\n");
	createFile("/dir1",1);
	printf("5. create a file named file2 in the directory.\n");
	createFile("/dir1/file2",0);
	printf("6. write Hello,world into file2.\n");
	writeFile("/dir1/file2","Hello,world");
	printf("7. read five bytes and print them: ");
	readFile("/dir1/file2",5,ptr);
	printf("%s\n",ptr);
	
	printf("8. create another file named file3 in the directory.\n");
	createFile("/dir1/file3",0);
	printf("	The inode number of file3 is %d.\n",findInode("/dir1/file3"));
	printf("9. delete file2.\n");
	deleteFile("/dir1/file2");
	printf("10. delete file3.\n");
	deleteFile("/dir1/file3");
	printf("11. delete directory.\n");
	deleteFile("/dir1");
	return 0;
}
