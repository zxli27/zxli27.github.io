#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "driver.h"
#include "File.h"

#define flat 0
#define directory 1

const char *fileAddr="vdisk";
const int NUM_INODES=128;


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
	memset(buffer,0,512);
	fp=fopen(fileAddr,"r+");
	//superblock
	int magicnum=42;              
	memcpy(buffer,&magicnum,4);    //????????????????????????
	memcpy(&buffer[4],&NUM_BLOCKS,4);
	memcpy(&buffer[8],&NUM_INODES,4);

	//fscanf(buffer,"%d%d%d",magicnum,NUM_BLOCKS,NUM_INODES);
	writeBlock(fp,0,buffer);
	//block1: free block vector
	memset(buffer,255,512);   //make every bit 1
	buffer[0]=0;
	buffer[1]=31;
	writeBlock(fp,1,buffer);
	//block2: inode map
	memset(buffer,0,512);
	for(int i=0;i<128;i++){
		buffer[i*3]=i;
	}
	buffer[1]=10;
	writeBlock(fp,2,buffer);
	//block10: set root directory inode
	inode root;
	root.fsize=0;
	root.ftype=directory;
	for(int i=0;i<10;i++){
		root.dirblocks[i]=0;
	}
	root.sib=0;
	root.dib=0;
	memset(buffer,0,512);
	inodeIntoArray(root,buffer);
	writeBlock(fp,10,buffer);
	fclose(fp);
}

int findInode(char *addr){
	if(strcmp(addr,"/")==0){
		return 0;
	}
	char map[512];
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");	
	readBlock(fp,2,map);
	int len=strlen(addr);
	char ad[len+1];
	ad[len]='\0';
	memcpy(ad,addr,len);
	char *token=strtok(ad,"/");	
	char fname[32]="";
	int inumber=0;
	int inode=0;
	int flag=1;
	
    while(token!=NULL){
		readBlock(fp,map[3*inode+1]+map[3*inode+2]*256,buffer);
		short num=1;
		char buffer2[512];
		for(int i=0;i<10 && num!=0;i++){
			
			num=buffer[8+2*i]+buffer[9+2*i]*256;
			readBlock(fp,(int)num,buffer2);
			for(int j=0;j<16;j++){
				inumber=buffer2[j*32];
				if(inumber==0){continue;}
				for(int k=1;k<32 ;k++){
					fname[k-1]=buffer2[j*32+k];
					if(fname[k-1]=='\0'){
						break;
					}
				}
				if(strncmp(fname,token,strlen(token))==0){
				
					inode=inumber;
					flag=0;
					break;
				}
				
			}
			if(flag==0){
				break;
			}
		}
		if(flag==1){
			fprintf(stderr,"The address is not correct.\n");
			fclose(fp);
			return -1;
		}
		flag=1;
		token=strtok(NULL,"/");
	}
	fclose(fp);
	return inode;
}

int findFreeInode(){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,2,buffer);
	for(int i=0;i<128;i++){
		if(buffer[i*3+1]==0 && buffer[3*i+2]==0){
			fclose(fp);
			return buffer[3*i];
		}
	}
	fclose(fp);
	return -1;
}

int findTail(){
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,1,buffer);
	for(int i=511;i>=0;i--){
		char c=0b11111110;
		if((unsigned char)buffer[i]!=255){
			for(int j=7;j>=0;j--){
				if(((unsigned char)(c|buffer[i]))!=255){
					fclose(fp);
					return i*8+j+1;
				}
				c=c<<1;
			}
		}
	}
	fclose(fp);
	return -1;
}

int createFile(char *addr,int type){
	char map[512];
	char vector[512];
	char segment[3][512];
	for(int i=0;i<3;i++){memset(segment[i],0,512);}
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,1,vector);
	readBlock(fp,2,map);
	//find free inode and block
	int inum=findFreeInode(fp);
	if(inum==-1){
		fprintf(stderr,"there is no avaliable inode for a new file.\n");
		fclose(fp);
		return -1;
	}
	int tail=findTail(fp);
	if(tail==-1){
		fprintf(stderr,"there is no avaliable block for a new file.\n");
		fclose(fp);
		return -1;
	}
	//find parent directory
	int i=strlen(addr)-1;
	while(addr[i]!='/'){
		i--;
	}
	int dirInode;
	if(i==0){
		dirInode=0;
	}
	else{
		char path[i+1];
		strncpy(path,addr,i);
		path[i]='\0';
		dirInode=findInode(path);
	}
	if(dirInode==-1){
		fprintf(stderr,"The path doesn't exist.\n");
		fclose(fp);
		return -1;
	}
	//update map and vector
	map[3*inum+1]=(tail+2)%256;
	map[3*inum+2]=(tail+2)/256;
	vector[(tail+2)/8]=vector[(tail+2)/8]-(0b10000000>>((tail+2)%8));
	vector[(tail+1)/8]=vector[(tail+1)/8]-(0b10000000>>((tail+1)%8));
	vector[tail/8]=vector[tail/8]-(0b10000000>>(tail%8));
	int pInodeBlock=map[3*dirInode+1]+map[3*dirInode+2]*256;
	map[3*dirInode+2]=tail/256;
	map[3*dirInode+1]=tail%256;
	vector[pInodeBlock/8]=vector[pInodeBlock/8]|(0b10000000>>(pInodeBlock%8));
	//add an entry to the directory
	readBlock(fp,pInodeBlock,segment[0]);
	int size=((int)segment[0][0])+(((int)segment[0][1])<<8)+(((int)segment[0][2])<<16)+(((int)segment[0][3])<<24);
	size+=32;
	memcpy(&segment[0][0],&size,4);
	
	char entry[strlen(addr)-i+1];
	entry[0]=inum;
	strncpy(&entry[1],&addr[i+1],strlen(addr)-i-1);
	entry[strlen(addr)-i]='\0';
	int flag=1;	
	int targetBlock=0;
	for(int k=0;k<10;k++){
		targetBlock=(int)(segment[0][8+k*2])+(int)(segment[0][9+k*2])*256;
		if(targetBlock==0){
			segment[0][8+k*2]=(tail+1)%256;
			segment[0][9+k*2]=(tail+1)/256;
			memcpy(segment[1],entry,strlen(addr)-i+1);
			break;
		}
		readBlock(fp,targetBlock,segment[1]);
		for(int a=0;a<16;a++){
			if(segment[1][a*32]==0){
				segment[0][8+k*2]=(tail+1)%256;
				segment[0][9+k*2]=(tail+1)/256;
				memcpy(&segment[1][a*32],entry,strlen(addr)-i+1);
				vector[targetBlock/8]=vector[targetBlock/8]+(0b10000000>>(targetBlock%8));
				flag=0;
				break;
			}
		}
		if(flag==0){break;}

	}
	//create a inode struct and put it into the block
	inode newfile;
	newfile.fsize=0;
	newfile.ftype=type;
	for(int i=0;i<10;i++){
		newfile.dirblocks[i]=0;
	}
	newfile.sib=0;
	newfile.dib=0;
	inodeIntoArray(newfile,segment[2]);
	//push segment to the tail of the log and update inode map and block vector
	writeBlock(fp,1,vector);
	writeBlock(fp,2,map);
	for(int i=0;i<3;i++){
		writeBlock(fp,tail+i,segment[i]);
	}


	fclose(fp);
	return 0;
}

int deleteFile(char *addr){
	char map[512];
	char vector[512];
	char segment[3][512];
	for(int i=0;i<3;i++){memset(segment[i],0,512);}
	char buffer[512];
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,1,vector);
	readBlock(fp,2,map);
	int tail=findTail(fp);
	if(tail==-1){
		fprintf(stderr,"there is no avaliable block for a new file.\n");
		fclose(fp);
		return -1;
	}
	//get the block numbers of the file
	int inodeNum=findInode(addr);
	if(inodeNum==-1){
		fclose(fp);
		return -1;
	}
	int inodeBlock=map[3*inodeNum+1]+map[3*inodeNum+2]*256;
	readBlock(fp,inodeBlock,buffer);
	//check if it is a directory
	if(buffer[4]==1){
		for(int i=0;i<4;i++){
			if(buffer[i]!=0){
				fprintf(stderr,"the directory is not empty.\n");
				fclose(fp);
				return -1;
			}
		}
	}
	int blocks[10];
	for(int i=0;i<10;i++){
		blocks[i]=buffer[9+2*i]*256+buffer[8+2*i];
	}
	//free the inode and the blocks
	int i=0;
	while(i<10&&blocks[i]!=0){
		vector[blocks[i]/8]=vector[blocks[i]/8]|(0b10000000>>(blocks[i]%8));
		i++;
	}
	vector[inodeBlock/8]=vector[inodeBlock/8]|(0b10000000>>(inodeBlock%8));
	vector[(tail+1)/8]=vector[(tail+1)/8]-(0b10000000>>((tail+1)%8));
	vector[tail/8]=vector[tail/8]-(0b10000000>>(tail%8));
	map[3*inodeNum+1]=0;
	map[3*inodeNum+2]=0;
	//remove the entry from the directory
	i=strlen(addr)-1;
	while(addr[i]!='/'){
		i--;
	}
	int dirInode=0;
	if(i!=0){
		char path[i+1];
		strncpy(path,addr,i);
		path[i]='\0';
		dirInode=findInode(path);
	}
	if(dirInode==-1){
		fprintf(stderr,"The path doesn't exist.\n");
		fclose(fp);
		return -1;
	}
	int pInodeBlock=map[3*dirInode+1]+map[3*dirInode+2]*256;
	vector[pInodeBlock/8]=vector[pInodeBlock/8]|(0b10000000>>(pInodeBlock%8));
	readBlock(fp,pInodeBlock,segment[0]);
	int size=((int)(segment[0][3])<<24)+((int)(segment[0][2])<<16)+((int)(segment[0][1])<<8)+(int)(segment[0][0]);
	size-=32;
	memcpy(&segment[0][0],&size,4);
	short num=1;
	for(int i=0;i<10 && num!=0;i++){
		num=segment[0][9+2*i]*256+segment[0][8+2*i];
		readBlock(fp,num,segment[1]);
		for(int j=0;j<16;j++){
			char inumber=segment[1][j*32];
			if(inumber==inodeNum){
				memset(&segment[1][j*32],0,32);
				segment[0][8+2*i]=(tail+1)%256;
				segment[0][9+2*i]=(tail+1)/256;
				map[3*dirInode+1]=tail%256;
				map[3*dirInode+2]=tail/256;
				vector[num/8]=vector[num/8]|(0b10000000>>(num%8));
				writeBlock(fp,1,vector);
				writeBlock(fp,2,map);
				for(int i=0;i<2;i++){
					writeBlock(fp,tail+i,segment[i]);
				}
				fclose(fp);
				return 1;
			}

		}
	}
	fclose(fp);
	return 0;
}

int readFile(char *addr){
	char buffer[512];
	char map[512];
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,2,map);
	int inum=findInode(addr);
	if(inum==-1){
		fprintf(stderr,"The file doesn't exist.\n");
		fclose(fp);
		return -1;
	}
	readBlock(fp,map[3*inum+1]+map[3*inum+2]*256,buffer);
	int size=((int)(buffer[3])<<24)+((int)(buffer[2])<<16)+((int)(buffer[1])<<8)+(int)(buffer[0]);
	int num=1;
	char buffer2[513];
	for(int i=0;i<=size/512;i++){
		num=buffer[9+2*i]*256+buffer[8+2*i];
		readBlock(fp,num,buffer2);
		if(i==size/512){
			buffer2[size-i*512]='\0';
		}
		else{buffer2[512]='\0';}
		printf("%s",buffer2);
	}
	printf("\n");
	fclose(fp);
	return 0;
}

int writeFile(char *addr,char *content){
	char map[512];
	char vector[512];
	char seg[2][512];
	for(int i=0;i<2;i++){memset(seg[i],0,512);}
	FILE *fp=fopen(fileAddr,"r+");
	readBlock(fp,1,vector);
	readBlock(fp,2,map);
	int tail=findTail(fp);

	int inum=findInode(addr);
	if(inum==-1){
		fprintf(stderr,"The file doesn't exist.\n");
		fclose(fp);
		return -1;
	}
	int inodeBlock=map[3*inum+1]+map[3*inum+2]*256;
	map[3*inum+1]=tail%256;
	map[3*inum+2]=tail/256;
	readBlock(fp,inodeBlock,seg[0]);
	int size=((int)(seg[0][3])<<24)+((int)(seg[0][2])<<16)+((int)(seg[0][1])<<8)+(int)(seg[0][0]);
	int num=seg[0][9+size/512*2]*256+seg[0][8+size/512*2];
	if(num!=0){
		vector[num/8]=vector[num/8]|(0b10000000>>(num%8));
		readBlock(fp,num,seg[1]);
	}
	int contentlen=strlen(content);
	if(contentlen+size>10*512){
		fprintf(stderr,"The content exceeds the volume of the file\n");
		fclose(fp);
		return -1;
	}
	//first segment
	int seg1=512-size%512;
	int segnum=0;
	if(contentlen<512-size%512){
		seg1=contentlen;
		segnum=1;
	}
	else{
		float a=(float)(contentlen-(512-size%512));
		segnum=ceil(a/512)+1;
	}
	char segment[1+segnum][512];
	for(int i=0;i<1+segnum;i++){memset(segment[i],0,512);}
	memcpy(segment[0],seg[0],512);
	memcpy(segment[1],seg[1],512);
	for(int i=0;i<segnum;i++){
		segment[0][8+size/512*2+2*i]=(tail+i+1)%256;
		segment[0][9+size/512*2+2*i]=(tail+i+1)/256;
	}
	memcpy(&segment[1][size%512],content,seg1);
	//other segments
	short blocknum[segnum];
	blocknum[0]=0;
	vector[tail/8]=vector[tail/8]-(0b10000000>>(tail%8));
	vector[(tail+1)/8]=vector[(tail+1)/8]-(0b10000000>>((tail+1)%8));
	for(int i=1;i<segnum;i++){
		blocknum[i]=tail+1+i;
		memcpy(&segment[0][8+size/512*2+2*i],&blocknum[i],2);
		int segsize=512;
		if(i==segnum-1){
			segsize=contentlen-(512-size%512)-512*(segnum-2);
		}
		memcpy(segment[i+1],&content[512-size%512+512*(i-1)],segsize);
		vector[(tail+1+i)/8]=vector[(tail+1+i)/8]-(0b10000000>>((tail+1+i)%8));
	}
	//update block vector and file size
	size+=contentlen;
	memcpy(&segment[0][0],&size,4);
	vector[inodeBlock/8]=vector[inodeBlock/8]|(0b10000000>>(inodeBlock%8));

	writeBlock(fp,1,vector);
	writeBlock(fp,2,map);
	for(int i=0;i<segnum+1;i++){
		writeBlock(fp,tail+i,segment[i]);
	}
	fclose(fp);
	return 0;

}
int main(){
	initDisk();
	createFile("/dir1",1);
	createFile("/file3",0);
	createFile("dir1/dir2",0);
	createFile("dir1/dir2/file1",0);
	createFile("/dir1/file2",0);
	deleteFile("dir1/dir2/file1");
	printf("%d\n",findInode("/dir1/dir2/file1"));
	char b[531];
	for(int i=0;i<530;i++){b[i]='w';}
	b[530]='\0';
	writeFile("/dir1/file2",b);
	return 0;
	}
