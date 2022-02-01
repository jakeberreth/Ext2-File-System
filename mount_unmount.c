
int my_mount(char* filesys, char* mtp) {
    //if no parameters, display disks
    
    MINODE* mip;
    MINODE* di;
    if(strlen(filesys) == 0) {
        printf("mounted disks: \n");
      for(int i = 0; i < 16; i++) {
        if(mtable[i].dev != 0) {
            printf("global dev=%d\n", dev);
          printf("%s\n", mtable[i].devName);
          printf("dev: %d\n", mtable[i].dev);
          printf("mount name: %s\n", mtable[i].mntName);
          mip = mtable[i].mntDirPtr;
          printf("dir pointer ino: %d\n", mip->ino);
          printf("dir pointer device: %d\n", mip->dev);
          di = mip->mptr;
        }

      }
           return 2;
    }

    //if already mounted
    for(int i = 0; i < 16; i++) {
        if(strcmp(filesys, mtable[i].devName)==0) {
        printf("already mounted\n");
        return;
        }
    }

    int DEV = open(filesys, O_RDWR);
    SUPER* sp;
    printf("fd=%d\n", DEV);
    char buf[BLKSIZE];
    get_block(DEV, 1, buf); 
    sp = (SUPER *)buf; 
    if (sp->s_magic != 0xEF53){ //check if the device is a valid file system
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        close(DEV);
        return;
    } 

    //mount point
    //how is it supposed to be mounted
    int ino = getino(mtp, DEV);
    printf("ino=%d\n", ino);
    mip = iget(root->dev, ino);
    //not getting mip properly
    printf("mode=%4x\n", mip->INODE.i_mode);

    //check busy
    // if(mip->refCount > 0) {
    //     printf("busy\n");
    //     return;
    // }

    //check DIR
    char s[100];
    sprintf(s, "%4x", mip->INODE.i_mode);
    if(s[0] != '4') {
        printf("is not a directory\n");
        close(DEV);
        return;
    }

    MTABLE* mt;
    //record new dev
    for(int i = 0; i < 16; i++) {
        printf("enter loop\n");
        if(mtable[i].dev == 0) {
               mt = &mtable[i];
                printf("found free spot\n");
                break;
            //how do I show the disk within mnt?
        }
    }

    mt->dev = DEV;

    ninodes = mt->ninodes = sp->s_inodes_count;

    nblocks = mt->nblocks = sp->s_blocks_count; 
    strcpy(mt->devName, filesys);
    strcpy(mt->mntName, mtp);

    mip->mounted = 1;
    mip->mptr = mt;
    mt->mntDirPtr = mip;

    get_block(DEV, 2, buf); //get group descriptor block (B2)
    gp = (GD *)buf; //caste buf to GD block struct type
    bmap = mt->bmap = gp->bg_block_bitmap; //set bmap and mp's bmap numberto gp's bitmap block number
    imap = mt->imap = gp->bg_inode_bitmap; //set imap and mp's imap number to gp's inode block number
    inode_start = mt->iblock = gp->bg_inode_table;
}

int my_unmount(char* filesys) {
    for(int i = 0; i < 16; i++) {
        if(strcmp(filesys, mtable[i].devName)==0 && mtable[i].dev !=0) {
          printf("mounted\n");
        }
    }
    for(int i = 0; i < NMINODE; i++) {
        if(minode[i].refCount > 0) {
            return;
        }
    }
    //minode[i].
    //MINODE* mip = iget(dev, ino);
}
