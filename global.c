/////////////////////////////////////////////////////
// Programers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Mount_root
// File: global.c
// Date Completed: 
/////////////////////////////////////////////////////

#include "type.h"

//globals
MINODE minode[NMINODE]; //array of 128 in memory INODES
MINODE *root; //root MINODE
int mount;

PROC   proc[NPROC], *running; //array of 2 proc structures, running process

char gpath[128]; //global for tokenized components
char *name[32];  //assume at most 32 components in pathname
int   n;         //number of component strings

int fd, dev; //file descriptor and device number
int nblocks, ninodes, bmap, imap, inode_start; //MTABLE structure variables

MTABLE mtable[NMTABLE];     //mount tables
OFT oft[NOFT];              //Opened file instance

int fs_init(){
    int i, j;
    for (i = 0; i < NMINODE; i++) {     // initializes all minodes as FREE
        minode[i].refCount = 0;
    }
    for (i = 0; i < NMTABLE; i++) {     // initializes mtables as FREE
        mtable[i].dev = 0;
    }
    for (i = 0; i < NOFT; i++){         // initalizes ofts as FREE
        oft[i].refCount = 0;
    }
    for (i = 0; i < NPROC; i++) {        // initialize PROCs
        proc[i].status = READY;         // ready to run
        proc[i].pid = i;                // pid = 0 to NPROC-1
        proc[i].uid = i;                // P0 is a superuser process
        for (j = 0; j < NFD; j++){
            proc[i].fd[j] = 0;          // all file descriptors are NULL
        }
        proc[i].next = &proc[i +1];     // link list
    }
    proc[NPROC - 1].next = &proc[0];     // circular list
    running = &proc[0];                 // P0 runes first
}

MINODE *mialloc(){                      // allocate a FREE minode for use
    int i;
    for (i = 0; i < NMINODE; i++){ //go through all minodes
        MINODE *mp = &minode[i]; //set minode pointer (so not currently in memory) to minode[i]
        if (mp -> refCount == 0){ //if has not been used yet, set use count to 1 and return mp
            mp -> refCount =1;
            return mp;
        }
    }
    printf("FS panic: out of minodes\n"); //no FREE minodes for allocation
}

int midalloc(MINODE *mip){              // release a used minode
    mip -> refCount = 0; //set use count to 0
}