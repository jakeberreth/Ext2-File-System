/////////////////////////////////////////////////////
// Programmers: Jake Berreth, Garth Bates, KC Wang
// Class: CptS 360
// Project: Ext2 File System
// Date Completed: May 2021
/////////////////////////////////////////////////////

#include <stdio.h> //standard input/output
#include <stdlib.h> //standard library
#include <fcntl.h> //file descriptor functionality
#include <ext2fs/ext2_fs.h> //ext2 file system
#include <string.h> //string operations
#include <libgen.h> //pattern matching functions
#include <sys/stat.h> //fstat() and stat() functions
#include <time.h> //time operations

int fileType = 0;

//file includes
#include "type.h" //include type.h - global vars, #define directive, and structure declarations
#include "util.c"
//#include "global.c"
#include "cd_ls_pwd.c"
#include "mkdir_creat.c"
#include "link_unlink.c"
#include "rmdir.c"
#include "open_close.c"
#include "read_cat.c"
#include "write_cp.c"
#include "mount_unmount.c"


char *disk; //define string for name of virtual disk

#include "symlink.c"

//initialize global data structures
int init()
{
  int i, j;

  for (i=0; i<NMINODE; i++){ //set use count of each minode to 0
    minode[i].refCount = 0; 
  }
  MTABLE* mt;
  for(i=0; i<NMTABLE; i++) { //initialize all mtables as FREE, 0 dev means FREE
    mt = &mtable[i];
    mt->dev = 0;
    mt->nblocks = 0;
    mt->ninodes = 0;
    bzero(mt->devName, 64);
    bzero(mt->mntName, 64);
    mt->free_blocks = 0;
    mt->free_inodes = 0;
    mt->iblock = 0;
    mt->imap = 0;
    mt->bmap = 0;
    mt->mntDirPtr = 0;
  }
  for (i=0; i<NPROC; i++){ //initialize all proc structures 
    proc[i].status = READY;
    proc[i].pid = i;
    proc[i].uid = i;
    for(j = 0; j <NFD; j++) { //initialize file descriptor of each proc to 0
      proc[i].fd[j] = 0;
    }
    proc[NPROC-1].next = &proc[0]; //circular list
    running = &proc[0]; //P0 is the first running process
  }
}

//mount the root file system at system initialization
int mount_root(char *disk)
{
  int i;
  MTABLE *mp;
  SUPER *sp;
  GD *gp;
  char buf[BLKSIZE];

  dev = open(disk, O_RDWR); //set device num to the returned file descriptor after opening the disk for read only
  if(dev < 0) { //if file descriptor is less than 0, the disk was not opened properly
    printf("can't open root device\n");
    exit(1);
  }

  get_block(dev, 1, buf); //get super block, offset = block num * BLKSIZE, super is B1, read block into buf (B0 is boot block)
  sp = (SUPER *)buf; //caste buf (contains super block bytes) to a pointer to a SUPER block struct type
  
  if (sp->s_magic != 0xEF53){ //check if the device is a valid file system
    printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
    exit(1);
  }  

  mp = &mtable[0]; //fill mount table mtable[0] with disk info
  ninodes = mp->ninodes = sp->s_inodes_count; //copy super block info into mtable[0]
  nblocks = mp->nblocks = sp->s_blocks_count; 
  strcpy(mp->devName, disk);
  strcpy(mp->mntName, "/");
  mp->dev = 2;
  //printf("dev of disk %d\n", mp->dev);

  get_block(dev, 2, buf); //get group descriptor block (B2)
  gp = (GD *)buf; //caste buf to GD block struct type
  bmap = mp->bmap = gp->bg_block_bitmap; //set bmap and mp's bmap numberto gp's bitmap block number
  imap = mp->imap = gp->bg_inode_bitmap; //set imap and mp's imap number to gp's inode block number
  inode_start = mp->iblock = gp->bg_inode_table; //set inode_start and mp's iblock to gp's inode start block number
  //printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);
  root = iget(dev, 2); //call iget with device fd and inode number to get the root minode
  mp->mntDirPtr = root; //set mp's mount directory pointer to root becuase mp points to first MTABLE
  //mp->dev = dev;
  root->mptr = mp; //set root's mount pointer to mp
  for(i = 0; i < NPROC; i++) { //set proc CWDs
    proc[i].cwd = iget(dev, 2); //set MINODE* cwd with inode number 2, so the ino is the same as root
  }
  //printf("mount : %s mount on / \n", disk);
  return 0;
}

//program starting point
int main(int argc, char *argv[ ])
{
  disk = argv[1];
  //printf("disk name %s\n", disk);
  int ino; //inode number
  char buf[BLKSIZE]; //buf of size 1024 bytes
  char line[128], cmd[32], pathname[128], newFile[128]; //for passing into command functions
  MTABLE *mp; //MTABLE pointer
  char bytes[64];
  int param1, param2, param3;

  if (argc > 1) //if there are 2 or more command line arguments, then the 2nd will be the name of the disk
    disk = argv[1];

  printf("*********************************\n"); //welcome display
  printf("WELCOME TO THIS EPIC FILE SYSTEM\n");
  printf("*********************************\n\n");

  init(); //call init()
  mount_root(disk); //call mount_root() passing in name of disk image file, "diskimage"

  while(1){ //loop to continue forever
    printf("INPUT COMMAND: [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|quit] "); //prompt user for a command
    fgets(line, 128, stdin); //get the user input and save to line
    line[strlen(line)-1] = 0; //set the last char in line to null to denote the endpoint of the c string

    if (line[0]==0) //if nothing is entered, return to beginning of while loop
       continue;
    pathname[0] = 0; //initialize pathname

    sscanf(line, "%s %s %s", cmd, pathname, newFile, bytes); //place line components into cmd, pathname, and newFile
    //printf("cmd=%s pathname=%s\n\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0) //command functions
       ls(pathname);
    if (strcmp(cmd, "cd")==0)
       chdir(pathname);
    if (strcmp(cmd, "pwd")==0) {
       pwd(running->cwd);
       printf("\n");
    }
    if (strcmp(cmd, "rmdir")==0) {
      rmdir(pathname);
    }    
    
    if(strcmp(cmd, "unlink") == 0){
      unlink(pathname);
    }
    if(strcmp(cmd, "mkdir")==0) {
      fileType = 1;
      mk_dir(pathname);
    }
    if(strcmp(cmd, "creat")==0) {
      fileType = 2;
      creat_file(pathname);
    }
    if(strcmp(cmd, "link")==0)
      link(pathname, newFile);
    if(strcmp(cmd, "symlink")==0)
      symlink(pathname, newFile);
    if (strcmp(cmd, "quit")==0)
       quit();
    if(strcmp(cmd, "open")==0) {
      open_file(pathname, newFile);
    }
    if(strcmp(cmd, "lseek")==0)
      my_lseek(pathname, newFile);
    if(strcmp(cmd, "read")==0) {
      sscanf(pathname, "%d", param1);
      sscanf(bytes, "%d", param2);
      read_file(param1, newFile, param2);
    }
    if(strcmp(cmd, "cat")==0) {
      cat_file(pathname);
    }
    if(strcmp(cmd, "cp") == 0){
      my_cp(pathname, newFile);
    }
    if(strcmp(cmd, "mount")==0) {
      my_mount(pathname, newFile);
    }
    if(strcmp(cmd, "unmount")==0)
      my_unmount(pathname);
  }
}

//quit the program
int quit()
{
  int i; 
  MINODE *mip; //in-memory inode
  for (i=0; i<NMINODE; i++){ //go through all minodes
    mip = &minode[i]; 
    if (mip->refCount > 0) //if the minode was used, put it back on the disk
      iput(mip); //put minode back on disk
  }
  exit(0); //exit system call to exit the program
}
