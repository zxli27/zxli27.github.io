

void initDisk();

int findInode(char *addr);

int createFile(char *addr,int type);

int deleteFile(char *addr);

int writeFile(char *addr,char *content);
       
int readFile(char *addr,int length,void *ptr);

int fsck();
