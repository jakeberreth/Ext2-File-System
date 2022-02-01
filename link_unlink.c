/////////////////////////////////////////////////////
// Programers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Mount_root
// File: util.c
// Date Completed: 
///////////////////////////////////////////////////// 

int link(char* oldFile, char* newFile) {
     //printf("in link\n");
    // printf("%s\n", oldFile);
    // // if(strcmp(oldFile, "")==0) {
    // //     putchar('f');
    // //     return;
    // // }
    //putchar('s');
    int oino = getino(oldFile, dev);
    //getchar();
    MINODE* omip = iget(dev, oino);
    int ino;
    char s[100];
    //getchar();
    sprintf(s, "%4x", omip->INODE.i_mode);
    //getchar();
    if(s[0] == '4') {
        //printf("Is a DIR\n");
        return;
    } else {
        ino = getino(newFile, dev);
        if(ino != 0) { //if it returns 0, then is doesn't
            printf("newFile already exists");
            return;
        } else {
            //printf("newFile does not exist\n");
            char* parent = dirname(newFile);
            char* child = basename(newFile);
            int pino = getino(parent, dev);
            MINODE* pmip = iget(dev, pino);
            enter_child(pmip, oino, child);
            omip->INODE.i_links_count++;
            omip->dirty = 1;
            iput(omip);
            iput(pmip);
        }
    }
}

int unlink(char *pathname){
    int block[15];
    int sym = 0;
    char buf[BLKSIZE], otherBuf[BLKSIZE];

    //printf("In unlink\n");

    //1 get filenames minode
    //printf("Step 1\n");
    int ino = getino(pathname, dev);
    MINODE* mip = iget(dev, ino);

    //check is a REG or symbolic LNK file; can not be a DIR
    if (S_ISDIR(mip -> INODE.i_mode)){
        //printf("Is a DIR\n");
        return -1;
    }

    //2 Remove mane entry from parent DIR's data block
    //printf("Step 2\n");
    char parent[256], child[256];
    strcpy(parent, dirname(pathname));
    strcpy(child, basename(pathname));

    int pino = getino(parent, dev);
    MINODE* pmip = iget(dev, pino);
    newRm_child(pmip, child);
    pmip->dirty = 1;
    iput(pmip);

    //3 Decrement INODE's link_count by 1
    ///printf("Step 3\n");
    mip->INODE.i_links_count--;

    //printf("Step 4\n");
    if(mip->INODE.i_links_count > 0){
        mip->dirty = 1;
    } else { //if link_count = 0; remove filename
        //printf("Step 5\n");
        for(int i = 0; i < 15; i++){
            block[i] = mip -> INODE.i_block[i];
        }

        if (!sym){

            //bdalloc direct blocks
            for(int i = 0; i < 12; i++){
                if(block[i]){
                    bdalloc(mip->dev, block[i]);
                }
            }

            //bdalloc indirect blocks
            if(block[12]){
                get_block(mip->dev, block[12], buf);
                int *k = (int *)buf;
                for(int i = 0; i < 256; i++){
                    if(*k){
                        bdalloc(mip->dev, *k);
                    }
                    k++;
                }
            }

            //bdalloc double indirect blocks
            if(block[13]){
                get_block(mip -> dev, block[13], buf);
                int *k =(int *)buf;
                for(int i = 0; i < 256; i++){
                    if(*k){
                        get_block(mip -> dev, *k, otherBuf);
                        int *g = (int *)otherBuf;
                        for (int j = 0; j < 256; j++){
                            if(*g){
                                bdalloc(mip -> dev, *g);
                            }
                            g++;
                            }
                    }
                    k++;
                }
                
            }
        }

        idalloc(mip -> dev, mip -> ino);

    }
}