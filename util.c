/////////////////////////////////////////////////////
// Programers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Mount_root
// File: util.c
// Date Completed: 
/////////////////////////////////////////////////////

#include "type.h"
#include "global.c"

MTABLE* get_mtable(int devVal) {
  MTABLE* mt = NULL;
  for(int i = 0; i < NMTABLE; i++) {
    if(mtable[i].dev == devVal) {
      mt = &mtable[i];
      return mt;
    }
  }
}

int get_block(int dev, int blk, char *buf){
  lseek(dev, blk*BLKSIZE, SEEK_SET); //takes file descriptor, offset, and SEEK_SET (offset in bytes), change pointer location
  int n = read(dev, buf, BLKSIZE); //read 1024 bytes from device number (fd) into buf, return num of bytes read
  if (n < 0) //if less than 0 bytes are read than there is a problem with getting the block from fd dev and block #blk
    printf("get_block [%d %d] error \n", dev, blk);
}

int put_block(int dev, int blk, char *buf){
  lseek(dev, blk*BLKSIZE, SEEK_SET); //takes fd, offset, and SEEK_SET
  //problem happens right here
  int n = write(dev, buf, BLKSIZE); //write 1024 bytes from fd into buf, which is block
  if (n != BLKSIZE) //if write does not return 1024 as the num of bytes written, error with writing to block
    printf("put_block [%d %d] error\n", dev, blk);
}

MINODE *iget(int dev, int ino){
  MINODE *mip; //in-memory inode pointer
  MTABLE *mp; //mount table pointer
  INODE *ip; //inode pointer
  int i, block, offset;
  char buf[BLKSIZE];

  for (i=0; i < NMINODE; i++){  //search in-memory minodes first
    MINODE *mip = &minode[i]; //set mip to i minode
    if (mip -> refCount && (mip -> dev == dev) && (mip -> ino == ino)){ //if use count is greater than 1, dev is correct, and inode number = ino
      mip -> refCount++; //add 1 to use count
      return mip; //return mip as the minode with the correct ino
    }
  }

  //needed INODE = (dev,ino) not in memory
  mip = mialloc(); // allocate a FREE minode to mip
  mip -> dev = dev; //ensure dev fd is correct
  mip -> ino = ino; //set mip's inode num to ino
  block = (ino - 1) / 8 + inode_start; //disk block containing the inode
  offset = (ino - 1) % 8; //set the offset
  get_block(dev, block, buf); //get block from the disk with block and offset
  ip = (INODE *)buf + offset; //caste buf to INODE* and add offset to find inode within the block
  mip -> INODE  = *ip; //set mip's inode to ip

  // initialize minode
  mip -> refCount = 1;
  mip -> mounted = 0;
  mip -> dirty = 0;
  mip -> mptr = 0;
  return mip;
}

void iput(MINODE *mip){
  INODE *ip; //inode pointer
  int i, block, offset; 
  char buf[BLKSIZE];

  if (mip == 0){ //if minode pointer is null, then there is nothing to put
    //printf("mip=0\n");
    return;
  }
  mip -> refCount--;                   //dec refCount by 1
  if (mip -> refCount > 0){            // still has user
    //printf("refcount\n");
    return;
  }
  if (mip -> dirty == 0){              // no need to write back
    //printf("dirty\n");
    return;
  }

  block = (mip -> ino - 1) / 8 + inode_start; //set block to block number of write back location
  offset = (mip -> ino -1) % 8; //set offset to location to write inode in block

  get_block(mip -> dev, block, buf); //get blockwhere inode will be written, size is 1024, fd is dev, block is the offset on the disk 
  ip = (INODE *)buf + offset;         //caste buf to INODE struct and add offset to get actual location for writing ip
  *ip = mip -> INODE;                 // copy INODE to inode in block
  put_block(mip -> dev, block, buf);  // write back to disk
  midalloc(mip);                      // mip -> refCount = 0;
}

int tokenize(char *pathname){
  char *s;
  strcpy(gpath, pathname);
  n = 0;
  s = strtok(gpath, "/");
  while (s) {
    name[n++] = s;
    s = strtok(0, "/");
  }
}

int search(MINODE *mip, char *name){
  //printf("name=%s\n");
  int i;
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;
    for(int i = 0; i < strlen(name); i++) {
          name[i] = tolower(name[i]);
      }
  for (i = 0; i < 12; i++){           // search DIR direct blocks only
    if (mip -> INODE.i_block[i] == 0){
      return 0;
    }
    get_block(mip -> dev, mip -> INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while (cp < sbuf + BLKSIZE){
      strncpy(temp, dp -> name, dp -> name_len);
        for(int i = 0; i < strlen(temp); i++) {
          temp[i] = tolower(temp[i]);
        }
        //printf("temp=%s\n");
      temp[dp -> name_len] = 0;
      //printf("%8d%8d%8u %s\n", dp -> inode, dp -> rec_len, dp -> name_len, temp);
      if (strcmp(name, temp) == 0){
        //printf("found %s : inumber = %d\n", name, dp -> inode);
        return dp -> inode;
      }
      cp += dp -> rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

int getino(char *pathname, int devName){
  //accept an MINODE, find ino of MINODE's PARENT
  MINODE *mip;
  int i, ino;
  INODE *ip;
  MTABLE *mn;
  int x = 0;

  if (strcmp(pathname, "/") == 0){
    printf("root\n");
    return 2;                   // return root ino=2
  }
  if (pathname[0] == '/'){
    //printf("/\n");
    mip = iget(dev, 2);
    printf("mip=%4x\n", mip->INODE.i_mode);   
    printf("mip ino=%d\n", mip->ino);          // if absolute pathname: start from root
  } else {
    mip = iget(running->cwd->dev, running->cwd->ino);
    // if relative pathname: start from cwd
  }
  mip -> refCount++;            // in order to iput(mip) later

  tokenize(pathname);           // assume: name[], nname are globals

  for (i = 0; i < n; i++){  // seach for each component string
    printf("in loop\n");
    if (!S_ISDIR(mip -> INODE.i_mode)){   // check DIR type
      //printf("%s is not a directory\n", name[i]);
      iput(mip);
      return 0;
    }
    printf("ino before search = %d\n", ino);
    ino = search(mip, name[i]);
    printf("ino  after search = %d\n", ino);
    if (!ino){
      //it gets no such component name in getino
      printf("no such component name %s\n", name[i]);
      iput(mip);
      return 0;
    }
    iput(mip);                  // release current minode
    mip = iget(dev, ino);       // switch to new minode
    printf("new ino=%d\n", ino); //supposed to be weird
    printf("mip mode=%4x\n", mip->INODE.i_mode);
    printf("root dev=%d\n", root->dev);
    printf("new dev=%d\n", dev);
    //mounted not registering as 1
    printf(" mip inode=%d, %d, %d\n", mip->ino, mip->dev, mip->mounted);
    if(ino == 2 && (dev != root->dev)) {
      printf("in if\n");
      printf("x=%d\n", x);
      if(x > 0) {
        printf("crossing mount points\n");
        mn = get_mtable(dev);
        mip = mn->mntDirPtr;
        dev = mip->dev;
      } 
              x++;
    } else if(mip->mounted) {
        mn = mip->mptr; //points to directories corresponding mount table
        if(dev != mn->dev) {
          printf("crosssing mount points\n");
          dev = mn->dev;
          mip = iget(dev, 2);
          ino = 2;
        }
      }
  
  }
  iput(mip);
  return ino;
}

int findmyname(MINODE *parent, u32 myino, char *myname){
  // search parent's data block for myino
  // copy its name STRING to myname[]

  int i;
  char buf[BLKSIZE];
  DIR *dp = (DIR *)buf;

  get_block(parent -> dev, parent -> INODE.i_block[0], buf);

  for (i = 0; i < BLKSIZE; i += dp -> rec_len){
    dp = (DIR *)(buf + i);

    if (dp -> rec_len <= 0){
      break;
    }
    strncpy(myname, dp -> name, dp -> name_len);
    myname[dp -> name_len] = 0;

    if (dp -> inode == myino){
      //printf("%s", myname);
      return;
    }
  }
  return 0;
}

int findino(MINODE *mip, u32 *myino){
  char buf[BLKSIZE], *cp;
  DIR *dp;

  get_block(mip -> dev, mip -> INODE.i_block[0], buf);
  cp = buf;
  dp = (DIR *)buf;
  *myino = dp -> inode;
  cp += dp -> rec_len;
  dp = (DIR *)cp;
  return dp -> inode;
}

int tst_bit(char *buf, int bit){
  return buf[bit/8] & (1 << (bit %8));
}

int set_bit(char *buf, int bit){
  buf[bit/8] |= (1 << (bit % 8));
}

int decFreeInodes(int dev)
{
  // dec free inodes count in SUPER and GD
  char buf[1024];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp -> s_free_inodes_count--;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp -> bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev){
  //dec free inodes count in SUPER and GD
  char buf[1024];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp -> s_free_blocks_count--;
  put_block(dev,1,buf);

  get_block(dev,2,buf);
  gp = (GD *)buf;
  gp -> bg_free_blocks_count--;
  put_block(dev,2,buf);
}

int ialloc(int dev){
  int i;
  char buf[BLKSIZE];
  // use imap, ninodes in mount table of dev
  //MTABLE *mp = (MTABLE *)get_mtable(dev)
  MTABLE* mp = NULL;
  for(int i = 0; i < NMTABLE; i++) {
    //printf("in loop\n");
    //printf("dev of mtable[%d]=%d\n", i, mtable[i].dev);
    if(mtable[i].dev == dev) {
      mp = &mtable[i];
      break;
    }
  }
  //printf("before getblock\n");
  get_block(dev, imap, buf);
  for(i = 0; i < ninodes; i++) {
    if(tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      put_block(dev, imap, buf);
      //update free inode count in SUPER and GD
      decFreeInodes(dev);
      return (i + 1);
    }
  }
  return 0;
}

/*
int balloc(int dev){
  int i;
  char buf[BLKSIZE];
  //use imap, ninodes in mount table of dev
  //MTABLE *mp = get_mtable(dev);
  MTABLE* mp = NULL;
  for(int i = 0; i < NMTABLE; i++){
    mp = &mtable[i];
    if(mp->dev == dev){
      break;
    }
  }
  //printf("after get_mtable\n");
  get_block(dev, bmap, buf);
  for( i = 0; i < ninodes; i++){
    if(tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      put_block(dev, bmap, buf);
      //update free inode count in SUPER and GD
      decFreeBlocks(dev);
      return (i + 1);
    }
  }
  return 0;
}
*/

// Allocates a free block
int balloc(int dev)
{
    int i;
    char buf[BLKSIZE];

    get_block(dev, bmap, buf);

    for(int i = 0; i < nblocks; i++)
    {
        if(tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            decFreeInodes(dev);
            put_block(dev, bmap, buf);

            printf("success balloc(): returns %d\n", i+1);
            return i+1;
        }
    }

    printf("balloc(): Error: could not allocate free block\n");

    return 0;
}

int clr_bit(char *buf, int bit){    // clear bit in char buf[BLKSIZE]
  buf[bit/8] &= ~(1 << (bit%8));
}

int incFreeInodes(int dev) {
  char buf[BLKSIZE];
  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp -> s_free_inodes_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp -> bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev){
  char buf[BLKSIZE];
  // inc free block coount in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp -> s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp -> bg_free_blocks_count++;
  put_block(dev, 2, buf);

}

int idalloc(int dev, int ino){
  int i;
  char buf[BLKSIZE];
  MTABLE *mp = (MTABLE *)get_mtable(dev);
  if (ino > ninodes){       // ninodes global
    printf("inumber &d out of range\n", ino);
    return;
  }
  // get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);
  // write buf back
  put_block(dev, imap, buf);
  // update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int bdalloc(int dev, int bno){
  char buf[BLKSIZE];

  get_block(dev, bmap, buf);
  clr_bit(buf, bno -1);
  put_block(dev, bmap, buf);
  incFreeBlocks(dev);
  return 0;
}

int rm_child(MINODE *parent, char *name){
  printf("In rm_child\n");
  int i;
  DIR *dp, *pdp, *ldp;
  char *cp, buf[BLKSIZE], *start, *end, tname[256], lcp;
  INODE *ip = &parent -> INODE;
  
  for (i = 0; i < 12; i ++) {
    if (ip -> i_block[i] != 0){
      get_block(parent -> dev, ip -> i_block[i], buf);
      dp = (DIR *)buf;
      cp = buf;

      while (cp < buf + BLKSIZE){     //not end of block
        // 1 Search parent INODE's data block(s) for the entry of name
        printf("rm child step 1\n");
        strncpy(tname, dp -> name, dp -> name_len); 
        tname[dp -> name_len] = 0; 

        if (!strcmp(tname, name)){
          //2.1 first and only entry in a data block
          printf("rm child step 2.1\n");
          if (cp == buf && cp + dp->rec_len == buf + BLKSIZE){ //segfault here
            printf("2.1 if statment\n");
            bdalloc(parent->dev, ip->i_block[i]);
            ip->i_size -= BLKSIZE;

            while (ip -> i_block[i + 1] != 0 && i + 1 < 12){
              printf("2.1 while loops");
              i++;
              get_block(parent -> dev, ip -> i_block[i], buf);
              put_block(parent -> dev, ip -> i_block[i - 1], buf);
              //i++;
            }
          }
        
          //2.2 LAST entry in block
          else if (cp + dp -> rec_len == buf + BLKSIZE){
            printf("rm child step 2.2\n");
            pdp -> rec_len += dp -> rec_len;
            put_block(parent -> dev, ip -> i_block[i], buf);
          }
          //2.3 entry is first but not the only entry or in the middle of a block
          else {
            printf("rm child 2.3\n");
            ldp = (DIR *)buf;
            lcp = buf;
          

            while (lcp + ldp -> rec_len < buf + BLKSIZE){
              lcp += ldp -> rec_len;
              ldp = (DIR *)lcp;
            }

            ldp -> rec_len += dp -> rec_len;

            start = cp + dp->rec_len;
            end = buf + BLKSIZE;

            memmove(cp, start, end - start);
            put_block(parent -> dev, ip -> i_block[i], buf);
        }

        parent -> dirty = 1;
        iput(parent);
        return 0;
      }

      pdp = dp;
      cp += dp -> rec_len;
      dp = (DIR *)cp;
      }
    }
  }
  printf("child not found \n");
  return -1;
}

int create_block(MINODE* pmip, int ino, int bno) {
  //printf("in create\n");
  char buf[1024];
  bzero(buf, BLKSIZE);
  get_block(dev, bno, buf); //get block from the disk with block and offset, call from balloc
  DIR *dp = (DIR *)buf;
  //make . entry
  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';
  dp->file_type = 2;
  //make .. entry: pino = parent DIR ino
  
  dp = (char *)dp + 12;
  dp->inode = pmip->ino;
  dp->rec_len = BLKSIZE - 12;
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  //don't think this is right
  dp->file_type = 2;
  //printf("bno=%d\n", bno);
  put_block(dev, bno, buf);
}

int load_inode(int ino) {
  //printf("in loadinode\n");
  MINODE* mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  if(fileType == 1) {
    ip->i_mode = 0x41ED;
    ip->i_links_count = 2;
    ip->i_size = BLKSIZE;
  } 
  else {
    ip->i_mode = 0x81a4;
    ip->i_links_count = 1;
    ip->i_size = 0;
  }

  ip->i_links_count = 2;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2;
  //don't think this is right
  ip->i_block[0] = 0;
  for(int i = 0; i < 14; i++) 
      ip->i_block[i] = 0;
  mip->dirty = 1;
  iput(mip);
}


int newRm_child(MINODE *parent, char *name){
  char buf[BLKSIZE], otherBuf[BLKSIZE];
  INODE* pip = &parent->INODE;
  DIR *temp, *prev;
  char *t;

  for(int i = 0; i < 12; i ++){
    if(pip->i_block[i] != 0){
      get_block(parent->dev, pip->i_block[i], buf);
      temp = (DIR *)buf;
      t = buf;
      prev = 0;
      while(t < buf + BLKSIZE){
        //printf("In while loop\n");
        bzero(otherBuf, 1024);
        strncat(otherBuf, temp->name, temp->name_len);
        if(strcmp(otherBuf, name) == 0){
          //2.1 first or last
          if(t + temp->rec_len == buf + BLKSIZE){
            if(prev == 0){
              //first
              //printf("First entry\n");
              bdalloc(parent -> dev, pip-> i_block[i]);
              pip -> i_size -= BLKSIZE;
              parent -> dirty = 1;

              while (pip -> i_block[i+1] != 0 && i + 1 < 12){
                i++;
                get_block(parent -> dev, pip -> i_block[i], buf);
                put_block(parent -> dev, pip -> i_block[i - 1], buf);
              }
              iput(parent);
              return 0;

            } else {
              //last
              //printf("Last entry \n");
              prev->rec_len += temp->rec_len;
              put_block(parent->dev, pip->i_block[i], buf);
              parent -> dirty = 1;
              iput(parent);
              return 0;
            }
          } else {
            // middle
            //printf("Middle Entry\n");
            //temp = (DIR *)buf;
            t = buf;
            DIR *d = buf;
            //printf("temp = %d\n", temp->inode);
            //getchar();
            
            while (t + d -> rec_len < buf + BLKSIZE){
              t += d -> rec_len;
              d = (DIR *)t;
            }
            //printf("After while loop\n");
            d -> rec_len += temp -> rec_len;

            char *start = (char *)temp + temp -> rec_len;
            int size = &buf[BLKSIZE] - start;

            //printf("Before memcpy\n");
            memcpy(temp, start, size); //segfault here
            //printf("After memcopy\n");
            put_block(parent -> dev, pip -> i_block[i], buf);

            return 0;
          }
        }
        prev = temp;
        t += temp->rec_len;
        temp = (DIR *)t;
      }
    } else {
      printf("Child not found\n");
      return -1;
    }
  }
}