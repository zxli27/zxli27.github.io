#include <stdio.h>
#include <stdlib.h>

#include "driver.h"

const int BLOCK_SIZE=512;
const int NUM_BLOCKS=4096;

void readBlock(FILE* disk, int blockNum, char* buffer){
	fseek(disk,blocknUm*BLOCK_SIZE,SEEK_SET);
	fread(buffer, BLOCK_SIZE, 1, disk);
}

void writeBlock(FILE* disk,int blockNum, char* data){
	fseek(disk,blocknUm*BLOCK_SIZE,SEEK_SET);
	fwrite(data,BLOCK_SIZE, 1, disk);
}