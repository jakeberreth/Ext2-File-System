/////////////////////////////////////////////////////
// Programers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Level 1
// File: rmdir.c
// Date Completed: 4/15/21
/////////////////////////////////////////////////////

int rmdir(char* pathname){
    //1 get in-memory INODE of pathname
    //printf("step 1\n");
    int ino = getino(pathname, dev);
    MINODE* mip = iget(dev, ino);

    //2 verify INODE is a DIR (by INODE.i_mode field)
    //printf("step 2\n");
    if (!S_ISDIR(mip -> INODE.i_mode)){
        printf("Not a DIR\n");
        return -1;
    }

    // minode is not BUSY (refcount = 1)
    if (mip -> refCount > 2 && fileType != 2){
        printf("Node is busy\n");
        printf("refCount: %d\n", mip->refCount);
        return -1;
    }

    if (!isEmpty(mip)){
        printf("DIR not empty\n");
        return -1;
    }

    //3 get parents ino and inode
    //printf("step 3\n");
    int pino = findino(mip, &ino); //get pino from .. entry in INODE.i_block[0]
    MINODE* pmip = iget(mip->dev, pino);

    //4 get name from parent DIRs data block
    //printf("step 4\n");
    char my_name[256];
    bzero(my_name, 256);
    findmyname(pmip, ino, my_name);
    //printf("%s\n", my_name);

    //5 remove name from parent directory
    //printf("step 5\n");
    //rm_child(pmip, my_name);
    newRm_child(pmip, my_name);

    //6 dec parent links_count by 1; mark parent pmip dirty
    //printf("step 6\n");
    pmip->INODE.i_links_count--;
    pmip->dirty = 1;
    iput(pmip);
    
    //7 deallocate its data blocks and inode
    //printf("step 7\n");
    bdalloc(mip->dev, mip->INODE.i_block[0]);
    idalloc(mip->dev, mip->ino);
    iput(mip);



}

int isEmpty(MINODE *mip) {
    char buf[BLKSIZE], *cp, tname[256];
    int i;
    DIR *dp;
    INODE *ip = &mip->INODE;

    if (ip -> i_links_count > 2){
        return 0; //more than the 2 default DIRs. 0 means not empty
    }
    else if (ip -> i_links_count == 2){         //checing if there are files
        for(i = 0; i < 12; i ++){
            if (ip -> i_block[i] == 0){
                break;
            }
            get_block(mip -> dev, mip -> INODE.i_block[i], buf);
            dp = (DIR *)buf;
            cp = buf;

            while (cp < buf + BLKSIZE){
                strncpy(tname, dp -> name, dp -> name_len);
                tname[dp -> name_len] = 0;
                if (strcmp(tname, ".") && strcmp(tname, "..")){
                    return 0; // there are more files
                }
                cp += dp -> rec_len; //move to next entry for next loop
                dp = (DIR *)cp;
            }
        }
    }

    return 1;  

}