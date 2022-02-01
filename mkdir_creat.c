/////////////////////////////////////////////////////
// Programmers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Mount_root
// File: util.c
// Date Completed: 
/////////////////////////////////////////////////////

extern int fileType;

int mk_dir(char* pathname) {
    char path[50];
    strcpy(path, pathname);
    //printf("%s\n", path);
    MINODE* start = running->cwd;
    char* parent = dirname(pathname);
    char* child = basename(pathname);
    int num = 0, count = 0;
    int i = 0, j = 0;
    for(i; i < strlen(path); i++) {
        //printf("%c\n", path[i]);
        if(path[i] == '/') 
            num++;
    }
    i = 0;
    //printf("%d\n", num);
    char newChild[50];
    if(num > 0) {
        while(count < num) {
            //printf("%c\n", path[i]);
            if(path[i] == '/') {
                count++;
            }
            i++;
        }
        //getchar();
        j = 0;
        while(i < strlen(path)) {
            //printf("i=%d strlen=%d\n", i, strlen(path));
            char c = path[i];
            newChild[j] = c;
            j++;
            //getchar();
            //printf("%s\n", newChild);
            //getchar();
            i++;
        }   
        newChild[j] = NULL;
    }
    //printf("%d\n", strlen(newChild));
    //printf("%c\n", newChild[3]);
    if(num > 0) {
           //printf("parent=%s child=%s\n", parent, newChild); 
    } else
    //printf("parent=%s child=%s\n", parent, child);
    
    if(child[0] == '.') {
        return;
    }
    int pino = getino(parent, dev);
    MINODE* pmip = iget(dev, pino);
    //printf("pino=%d\n", pino);
    char s[100];
    if(fileType == 1) {
        sprintf(s, "%4x", pmip->INODE.i_mode);
        if(s[0] == '4') {
            //printf("pmip is a directory\n");
        } else {
            //printf("pmip is not a directory\n");
            return;
        }
    }
    int basenameExists = 0;
    if(num > 0)
        basenameExists = search(pmip, newChild);
    else
        basenameExists = search(pmip, child);
    if(basenameExists == 0) {
        //printf("basename does not exist in parent DIR\n");
    } else {
        //printf("basename does exist in parent DIR\n");
        return;
    }
    if(num > 0)
        my_mkdir(pmip, newChild, pino);
    else        
        my_mkdir(pmip, child, pino);
}

// void my_mkdir(MINODE* pmip, char* newFile, int pino) {
//     //allocate an INODE and a disk block
//     printf("dev=%d\n", dev);
//     int ino = ialloc(dev);
//     int blk = 0;
//     if(fileType == 1) {
//         blk = balloc(dev); //or bno
//     }
//     printf("%d %d\n", ino, blk);
    
//     if(fileType == 3) {
//         load_inode_symlink(ino);
//     } else {
//     //load INODE into a minode
//         load_inode(ino);
//     }

//     //make data block 0 of INODE contain . and .. entries
//     create_block(pmip,ino, blk);

//     MINODE* mip = iget(dev, ino);
//     mip->INODE.i_block[0] = blk;
//     mip->dirty = 1;
//     iput(mip);
//     //enter_child(pmip, ino, basename)
//     enter_child(pmip, ino, newFile);
// }

void creat_file(char* pathname) {
    char path[50];
    strcpy(path, pathname);
    //printf("%s\n", path);
    MINODE* start = running->cwd;
    char* parent = dirname(pathname);
    char* child = basename(pathname);
    int num = 0, count = 0;
    int i = 0, j = 0;
    for(i; i < strlen(path); i++) {
        //printf("%c\n", path[i]);
        if(path[i] == '/') 
            num++;
    }
    i = 0;
    //printf("%d\n", num);
    char newChild[50];
    if(num > 0) {
        while(count < num) {
            //printf("%c\n", path[i]);
            if(path[i] == '/') {
                count++;
            }
            i++;
        }
        //getchar();
        j = 0;
        while(i < strlen(path)) {
            //printf("i=%d strlen=%d\n", i, strlen(path));
            char c = path[i];
            newChild[j] = c;
            j++;
            //getchar();
            //printf("%s\n", newChild);
            //getchar();
            i++;
        }   
        newChild[j] = NULL;
    }
    //printf("%d\n", strlen(newChild));
    //printf("%c\n", newChild[3]);
    if(num > 0) {
           //printf("parent=%s child=%s\n", parent, newChild); 
    } else
    //printf("parent=%s child=%s\n", parent, child);
    
    if(child[0] == '.') {
        return;
    }
    int pino = getino(parent, dev);
    MINODE* pmip = iget(dev, pino);
    //printf("pino=%d\n", pino);
    char s[100];
    int basenameExists = 0;
    if(num > 0)
        basenameExists = search(pmip, newChild);
    else
        basenameExists = search(pmip, child);
    if(basenameExists == 0) {
        //printf("basename does not exist in parent DIR\n");
    } else {
        //printf("basename does exist in parent DIR\n");
        return;
    }
    if(num > 0)
        my_creat(pmip, newChild, pino);
    else        
        my_creat(pmip, child, pino);
}

void my_creat(MINODE* pmip, char* newFile, int pino) {
    //printf("dev=%d\n", dev);
    int ino = ialloc(dev);

    load_inode(ino);

    MINODE* mip = iget(dev, ino);
    mip->INODE.i_block[0] = 0;
    mip->dirty = 1;
    iput(mip);
    //enter_child(pmip, ino, basename)
    enter_child(pmip, ino, newFile);

}

void my_mkdir(MINODE* pmip, char* newFile, int pino) {
    //allocate an INODE and a disk block
    //printf("dev=%d\n", dev);
    int ino = ialloc(dev);
    int blk = balloc(dev); //or bno
    //printf("%d %d\n", ino, blk);

    load_inode(ino);

    //make data block 0 of INODE contain . and .. entries
    create_block(pmip,ino, blk);

    MINODE* mip = iget(dev, ino);
    mip->INODE.i_block[0] = blk;
    mip->dirty = 1;
    iput(mip);
    //enter_child(pmip, ino, basename)
    enter_child(pmip, ino, newFile);
}

int enter_child(MINODE* pmip, int ino, char* newName) {
  char buf[BLKSIZE];
  char* cp;
  int idealLength = 0, nL = 0;
  int bno;
  int totalLength = 0;
  int nLen = strlen(newName);

  int needLength = 4 * ((8 + nLen + 3) / 4);

  for(int i=0; i < 12; i++) {
    //printf("i=%d\n", i);
    if(pmip->INODE.i_block[i] == 0) {
      break;
    } else {
      bno = pmip->INODE.i_block[i];
    }

    get_block(dev, bno, buf);
    dp = (DIR *)buf;
    cp = buf;
    //printf("name: %s\n", dp->name);
    totalLength = dp->rec_len;
    while(cp + dp->rec_len < buf + BLKSIZE) {
      cp += dp->rec_len;
      dp = (DIR *)cp;
      totalLength += dp->rec_len;
      //printf("%s\n", dp->name);
    }
    //printf("total: %d\n", totalLength);
    //printf("dp points at last entry\n");
    //printf("%d\n", dp->inode);
    int name_len = strlen(dp->name);
    idealLength = 4 * ((8 + name_len + 3) / 4);
    int remain = dp->rec_len - idealLength;
    if(remain > needLength) {
      dp->rec_len = idealLength;
      cp += idealLength;
      dp = (DIR *)cp;
      int totalFilled = (unsigned int)cp - (unsigned int)buf;
      int left = BLKSIZE - totalFilled;
      strcpy(dp->name, newName);
      dp->rec_len = left;
      //printf("name=%s\n", dp->name);
      dp->name_len = nLen;
      dp->inode = ino;
      dp->file_type = 2;
      put_block(dev, bno, buf);
      return;
    }
  }
    // pmip->INODE.i_links_count++;
    // pmip->dirty = 0; //set to dirty
}