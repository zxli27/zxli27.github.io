#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../disk/driver.h"
#include "File.h"

#define flat 0
#define directory 1

const fileAddr="../disk/vdisk";
const NUM_INODES=112;


typedef struct node{
	int fsize;
	int ftype;
	short dirblocks[10];
	short sib;
	short dib;
} inode;

void inodeIntoArray(inode a,char *start){
	memcpy(start,&a.fsize,4);
	memcpy(start+4,&a.ftype,4);
	for(int i=0;i<10;i++){
		memcpy(start+8+i*2,&a.dirblocks[i],2);
	}
	memcpy(start+28,&a.sib,2);
	memcpy(start+30,&a.dib,2);
}

void initDisk(){
	char buffer[512];
//wipe clean a file and set its size to 512*4096
	FILE *fp=fopen(fileAddr,"w");
	char *init=calloc(BLOCK_SIZE*NUM_BLOCKS,1);
	fwrite(init,BLOCK_SIZE*NUM_BLOCKS,1,fp);
	free(init);
	fclose(fp);
//format
	fp=fopen(fileAddr,"r+");
	//superblock
	int magicnum=42;              //???????????????
	fscanf(buffer,"%d%d%d",magicnum,NUM_BLOCKS,NUM_INODES);
	writeBlock(fp,0,buffer);
	//block1: free block vector
	memset(buffer,255,512);   //make every bit 1
	buffer[0]=0;
	buffer[1]=63;
	writeBlock(fp,1,buffer);
	//block2: free inode vector
	memset(buffer,255,14);   
	buffer[0]=127;             //root directory inode is inavaliable
	writeBlock(fp,2,buffer);
	//block3: set root directory inode
	inode root;
	root.fsize=0;
	root.ftype=directory;
	root.dirblocks[0]=10;
	for(int i=1;i<10;i++){
		root.dirblocks[i]=0;
	}
	root.sib=0;
	root.dib=0;
	memset(buffer,0,512);
	inodeIntoArray(root,buffer);
	writeBlock(fp,3,buffer);
}

int findInode(char *addr){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	char *token=strtok(addr,"/");
	char fname[32]="";
	int parentblock=3;
	int blocknum=3;
	int inode=0;
	int flag=1;
	while(token!=NULL){
		readBlock(fp,blocknum,buffer);
		short num=1;
		char buffer2[512];
		for(int i=0;i<10 && num!=0;i++){
			num=(buffer[inode%16*32+8+2*i]-'0')*256+(buffer[inode%16*32+9+2*i]-'0'); //??????????????????????
			readBlock(fp,(int)num,buffer2);
			for(int j=0;j<16;j++){
				char inumber=buffer2[j*32];
			
				for(int k=1;k<32 ;k++){
					fname[k-1]=buffer2[j*32+k];
					if(fname[k-1]=='\0'){
						break;
					}
				}
				if(strcmp(fname,token)==0){
					inode=inumber;
					parentblock=blocknum;
					blocknum=inumber;
					flag=0;
					break;
				}

			}
			if(flag==0){
				flag=1;
				break;
			}
		}
		if(blocknum==parentblock){
			return -1;
		}
		token=strtok(NULL,"/");
	}
	return blocknum;
}

int findFreeInode(){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,2,buffer);
	for(int i=0;i<14;i++){
		char c=0b100000000;
		for(int j=0;j<8;j++){
			if(c&buffer[i]!=0){
				return i*8+j;
			}
			c=c>>1;
		}
	}
	return -1;
}

short findFreeBlock(){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,1,buffer);
	for(int i=0;i<512;i++){
		char c=0b10000000;
		for(int j=0;j<8;j++){
			if(c&buffer[i]!=0){
				return short(i*8+j);
			}
			c=c>>1;
		}
	}
	return -1;
}

int createFile(char *addr,int type){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	//find free inode and block
	int inum=findFreeInode(fp);
	if(inum==-1){
		fprintf(stderr,"there is no avaliable inode for a new file.\n");
		return -1;
	}
	int blocknum=findFreeBlock(fp);
	if(blocknum==-1){
		fprintf(stderr,"there is no avaliable block for a new file.\n");
		return -1;
	}
	//find the path
	int i=strlen(addr)-1;
	while(addr[i]!='/'){
		i--;
	}
	char path[i+1];
	strncpy(path,addr,i);
	path[i]='\0';
	int dirInode=findInode(fp,path);
	if(dirInode==-1){
		fprintf(stderr,"The path doesn't exist.\n");
		return -1;
	}
	//update the free inode/block vector block
	readBlock(fp,1,buffer);
	buffer[blocknum/8]=buffer[blocknum/8]-(0b10000000>>(blocknum%8));
	writeBlock(fp,1,buffer);

	readBlock(fp,2,buffer);
	buffer[inum/8]=buffer[inum/8]-(0b10000000>>(inum%8));
	writeBlock(fp,2,buffer);
	//add an entry to the directory
	readBlock(fp,3+dirInode/16,buffer);
	int size=int(buffer[dirInode%16*32])<<24+int(buffer[dirInode%16*32+1])<<16+int(buffer[dirInode%16*32+2])<<8+int(buffer[dirInode%16*32+3]);
	size+=32;
	memcpy(&buffer[dirInode%16*32],size,4);
	size-=32;
	//what if the block is full??????????????
	//???????????????????????????????????
	int targetBlock=int(buffer[dirInode%16*32+8+size/256])<<8+int(buffer[dirInode%16*32+8+size/256+1]);
	readBlock(fp,targetBlock,buffer);
	char entry[strlen(addr)-i+1];
	entry[0]=inum;
	strncpy(&entry[1],&addr[i+1],strlen(addr)-i-1);
	entry[strlen(addr)-i]='\0';

	memcpy(&buffer[size%512],entry,strlen(addr)-i+1);
	writeBlock(fp,targetBlock,buffer);

	//create a inode struct and put it into the block
	inode newfile;
	newfile.fsize=0;
	newfile.ftype=type;
	newfile.dirblocks[0]=(short)blocknum;
	for(int i=1;i<10;i++){
		newfile.dirblocks[i]=0;
	}
	newfile.sib=0;
	newfile.dib=0;
	readBlock(fp,int(3+inum/16),buffer);
	inodeIntoArray(newfile,&buffer[inum%16*32]);
	writeBlock(fp,int(3+inum/16),buffer);
}

int deleteFile(char *addr){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	//get the block numbers of the file
	int inodeNum=findInode(fp,addr);
	if(inodeNum==-1){
		return -1;
	}
	readBlock(fp,3+inodeNum/16,buffer);
	//check if it is a directory
	if(buffer[inodeNum%16*32+7]==1){
		for(int i=0;i<4;i++){
			if(buffer[inodeNum%16*32+i]!=0){
				fprintf(stderr,"the directory is not empty.\n");
				return -1;
			}
		}
	}
	int blocks[10];
	for(int i=0;i<10;i++){
		blocks[i]=buffer[inodeNum%16*32+8+2*i]*256+buffer[inodeNum%16*32+9+2*i];
	}
	//free the inode and the blocks
	readBlock(fp,1,buffer);
	int i=0;
	while(blocks[i]!=0){
		buffer[blocks[i]/8]=buffer[blocks[i]/8]|(0b10000000>>(blocks[i]%8));
	}
	writeBlock(fp,1,buffer);
	readBlock(fp,2,buffer);
	buffer[inodeNum/8]=buffer[inodeNum/8]|(0b10000000>>(inodeNum%8));
	writeBlock(fp,2,buffer);
	//remove the entry from the directory
	int i=strlen(addr)-1;
	while(addr[i]!='/'){
		i--;
	}
	char path[i+1];
	strncpy(path,addr,i);
	path[i]='\0';
	int dirInode=findInode(fp,path);
	if(dirInode==-1){
		fprintf(stderr,"The path doesn't exist.\n");
		return -1;
	}
	readBlock(fp,3+dirInode/16,buffer);
	int size=int(buffer[dirInode%16*32])<<24+int(buffer[dirInode%16*32+1])<<16+int(buffer[dirInode%16*32+2])<<8+int(buffer[dirInode%16*32+3]);
	size-=32;
	memcpy(&buffer[dirInode%16*32],size,4);
	writeBlock(fp,3+dirInode/16,buffer);
	short num=1;
	char buffer2[512];
	for(int i=0;i<10 && num!=0;i++){
		num=buffer[dirNode%16*32+8+2*i]*256+buffer[dirNode%16*32+9+2*i];
		readBlock(fp,(int)num,buffer2);
		for(int j=0;j<16;j++){
			char inumber=buffer2[j*32];
			if(inumber==inodeNum){
				memset(&buffer2[j*32],0,32);
				writeBlock(fp,(int)num,buffer2);
				return 1;
			}

		}
	}
}

int readFile(char *addr){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	int inum=findInode(fp,addr);
	if(inum==-1){
		fprintf(stderr,"The file doesn't exist.\n");
		return -1;
	}
	readBlock(fp,3+inum/16,buffer);
	int num=1;
	char buffer2[513];
	buffer2[512]='\0';
	for(int i=0;i<10 && num!=0;i++){
		num=buffer[inum%16*32+8+2*i]*256+buffer[inum%16*32+9+2*i];
		readBlock(fp,num,buffer2);
		printf("%s",buffer2);
	}
}

int writeFile(char *addr,char *content){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	int inum=findInode(fp,addr);
	if(inum==-1){
		fprintf(stderr,"The file doesn't exist.\n");
		return -1;
	}
	readBlock(fp,3+inum/16,buffer);
	int size=int(buffer[inum%16*32])<<24+int(buffer[inum%16*32+1])<<16+int(buffer[inum%16*32+2])<<8+int(buffer[inum%16*32+3]);
	int num=buffer[inum%16*32+8+size/512*2]*256+buffer[inum%16*32+9+size/512*2];
	char *buffer2[512];
	readBlock(fp,num,buffer2);
	int contentlen=strlen(content);
	//first segment
	int seg1=512-size%512;
	int segnum=0;
	if(contentlen<512-size%512){
		seg1=contentlen;
		segnum=1;
	}
	else{
		segnum=math.ceil(((float)(contentlen-(512-size%512)))/512)+1;
	}
	memcpy(&buffer2[size%512],content,seg1);
	writeBlock(fp,num,buffer2);
	//other segments
	short blocknum[segnum];
	blocknum[0]=0;
	for(int i=1;i<segnum;i++){
		blocknum[i]=findFreeBlock();
		memcpy(&buffer[inum%16*32+8+size/512*2+2*i],&blocknum[i],2);
		readBlock(fp,blocknum[i],buffer2);
		int segsize=512;
		if(i==segnum-1){
			segsize=contentlen-(512-size%512)-512*(segnum-2);
		}
		memcpy(buffer2,&content[512-size%512+512*(i-1)],segsize);
		writeBlock(fp,blocknum[i],buffer2);
	}
	//update block vector and file size
	size+=contentlen;
	memcpy(buffer[&inum%16*32],size,4);
	writeBlock(fp,3+inum/16,buffer);
	readBlock(fp,1,buffer);
	for(int i=1;i<segnum;i++){
		buffer[blocknum[i]/8]=buffer[blocknum[i]/8]-(0b10000000>>(blocknum[i]%8));
	}
	writeBlock(fp,1,buffer);


}

int main(){
	return 0;
}

