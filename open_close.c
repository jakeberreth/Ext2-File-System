//open

int open_file(char* filename, char* flagString) {
    char c = flagString[0];
    int flag = c - '0';
    //printf("flag=%d\n", flag);
    //printf("in open\n");
    int ino = getino(filename, dev);
    //printf("ino=%d\n", ino);
    if(ino == 0) {
        //printf("file does not exist yet\n");
        creat_file(filename);
        ino = getino(filename, dev);
        //printf("ino=%d\n", ino);
    }
    MINODE* mip = iget(dev, ino);
    //have to allocate to disk
    OFT* poft = (OFT*)malloc(sizeof oft);
    poft->mode = flag;
    poft->mptr = mip;
    poft->refCount = 1;
    if(flag == 3) {
        //printf("flag = 3\n");
        poft->offset = mip->INODE.i_size; //end of file to append
    } else {
        //printf("flag not 3\n");
        poft->offset = 0;
    }
    int i = 0;
    for(i; i < 16; i++) {
        if(running->fd[i] == NULL) {
            //printf("found free fd\n");
            running->fd[i] = poft;
            //printf("%d\n", running->fd[i]->offset);
            break;
        }
    }
    //printf("index=%d\n", i);
    return i;
}

int my_lseek(char* fdStr, char* positionStr) {
    char c = fdStr[0];
    int fd = c - '0';
    int pos = 0;
    int multiplier = 100;
    //printf("fd=%d\n", fd);
    //printf("posstr=%s\n", positionStr);
    int l = strlen(positionStr);
    for(int i = 0; i < l; i++) {
        c = positionStr[i];
        int p = c - '0';
        pos += (p * multiplier);
        //printf("pos=%d\n", pos);
        multiplier /= 10;
    }
    //printf("pos=%d\n", pos);
    for(int i = 0; i < 16; i++) {
        if(running->fd[i] != NULL && i == fd) {
            //printf("opened fd at index=%d\n", i);
            running->fd[i]->offset = pos;
            //printf("new offest = %d\n", running->fd[i]->offset);
        }
    }
}

//int close_file(char* fdStr) {
int close_file(int fd){
    //  char c = fdStr[0];
    //int fd = c - '0';

    //printf("close fd=%d\n", fd); getchar();
    
    if(running->fd[fd] != NULL) {
        running->fd[fd]->refCount--;
        if(running->fd[fd]->refCount == 0) {
            iput(running->fd[fd]->mptr);
        }
    }
    running->fd[fd] = 0;
}
