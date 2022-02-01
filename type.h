/////////////////////////////////////////////////////
// Programers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Mount_root
// File: type.h
// Date Completed: 
/////////////////////////////////////////////////////

#ifndef TYPE_H_
#define TYPE_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>



typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp; //suberblock struct pointer
GD    *gp; //group descriptor struct pointer
INODE *ip; //inode struct pointer
DIR   *dp; //directory struct pointer

// Default dir and regular file mode
#define DIR_MODE    0x41ED
#define FILE_MODE   0x81AE
#define SUPER_MAGIC 0xEF53
#define SUPER_USER  0

// Proc status
#define FREE        0
#define READY       1

// file system table sizes
#define NMTABLE    16
#define BLKSIZE  1024
#define NMINODE   128
#define NFD        16
#define NPROC       2
#define NOFT       32

// In-memory inodes structure
typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mntable *mptr;
}MINODE;

// Open File Table
typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

// PROC structure
typedef struct proc{
  struct proc *next;
  int          pid;
  int          ppid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

// Mount Table structure
typedef struct mtable{
  int dev;            // device number
  int ninodes;        // from superblock
  int nblocks;
  int free_blocks;    // from superblock and GD
  int free_inodes;
  int bmap;           // from group descriptor
  int imap;
  int iblock;         // inodes start block
  MINODE *mntDirPtr;  // mount point DIR pointer
  char devName[64];   // device name
  char mntName[64];   // mount poit DIR name
}MTABLE;

#endif