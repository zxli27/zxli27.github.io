#include <stdio.h>
#include <stdlib.h>

const int BLOCK_SIZE;
const int NUM_BLOCKS;


void readBlock(FILE* disk, int blockNum, char* buffer);

void writeBlock(FILE* disk,int blockNum, char* data);
