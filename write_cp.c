/////////////////////////////////////////////////////
// Programers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Level 2
// File: wrtie_cp.c
// Date Completed: 
/////////////////////////////////////////////////////

#include "type.h"

int write_file(){
    //Step 1
    int fd = 0, nbytes = 0;
    char buf[BLKSIZE];
    printf("Enter file descriptor: ");
    scanf("%d", &fd);
    printf("Enter a string: ");
    scanf("%s", &buf);

    //Step 2
    if (fd != 1 || fd != 2 || fd != 3){
        printf("Error: Invalid file descriptor\n");
        return -1;
    }

    //Step 3
    nbytes = sizeof(buf);
    return mywrite(fd, buf, nbytes);


}

int mywrite(int fd, char buf[], int nbytes){
    //Step 1
    OFT *poft = running -> fd[fd];
    MINODE *mip = poft ->mptr;
    INODE *ip = &mip -> INODE;
    int count = 0, lblk, blk, start, remain = 0, doubleBLK, doubleIndex, doubleOffset;
    char *cp, *cq = buf, kbuf[BLKSIZE], wbuf[BLKSIZE];
    int dBuf[256],  doubleIBuf[256], bno, ibuf[256];


    //Step 2
    while(nbytes > 0){
        lblk = poft -> offset / BLKSIZE;    //compute logical block
        start = poft -> offset % BLKSIZE;   //computer start byte

        //Step 3
        if (lblk < 12) {    // direct block
            if(ip -> i_block[lblk] == 0){
                mip -> INODE.i_block[lblk] = balloc(mip -> dev);
            }
            blk = mip -> INODE.i_block[lblk];
        } else if (lblk >= 12 && lblk < 12 + 256){  //indirect block
            if(ip -> i_block[12] == 0 && lblk == 12){
                ip -> i_block[12] = balloc(mip -> dev);
                get_block(mip -> dev, ip->i_block[12], ibuf);
                ibuf[0] = balloc(mip -> dev);
                blk = ibuf[0];
                put_block(mip -> dev, ip -> i_block[12], ibuf);
            } else {
                get_block(mip -> dev, ip -> i_block[12], ibuf);
                blk = ibuf[lblk - 12];
                if(blk == 0){
                    ibuf[lblk - 12] = balloc(mip->dev);
                }

                //blk = ibuf[lblk - 12];
                put_block(mip -> dev, ip -> i_block[12], ibuf);
            }
        } else{ // double indirect block
            lblk = lblk - (12 + 256);

            // mailmans alg
            doubleIndex = (lblk - 268) / 256;
            doubleOffset = (lblk - 268) % 256;
            
            //if block doesnt exist, allocate
            if (ip -> i_block[13] == 0){
                ip -> i_block[13] = balloc(mip->dev);
                char newbuf[BLKSIZE];
                bzero(newbuf, BLKSIZE);
                put_block(mip -> dev, ip -> i_block[13], newbuf);
            }

            get_block(mip -> dev, ip -> i_block[13], (char *)doubleIBuf);
            int dblock = doubleIBuf[lblk / 256];

            // if double block is 0, allocate
            if (dblock == 0){
                dblock = balloc(mip -> dev);
                doubleIBuf[lblk/256] = dblock;
                char otherbuf[BLKSIZE];
                bzero(otherbuf, BLKSIZE);
                put_block(mip -> dev, dblock, otherbuf);
                put_block(mip -> dev, ip -> i_block[13], (char *)doubleIBuf);
            }
            get_block(mip -> dev, dblock, (char *)dBuf);

            if(dBuf[lblk % 256] == 0){
                bno = dBuf[lblk % 256] = balloc(mip -> dev);
                put_block(mip -> dev, dblock, (char *)dBuf);
            }
            blk = bno;
            printf("Double indect blk # %d\n", bno);

            /*
            // sets block to indicect block number that double inidect block points to
            doubleBLK = doubleIBuf[doubleIndex];

            put_block(mip -> dev, ip -> i_block[13], (char *)doubleIBuf);   //writes buf back
            get_block(mip -> dev, doubleBLK, (char *)doubleIBuf);           // get double indirect block into buf

            if (doubleIBuf[doubleOffset] == 0){
                doubleIBuf[doubleOffset] = balloc(mip -> dev);
            }

            blk = doubleIBuf[doubleOffset];

            put_block(mip -> dev, doubleBLK, (char * )doubleIBuf);
            */

        }

        //Step 4
        get_block(mip -> dev, blk, wbuf);          // read blk into wbuf
        cp = wbuf + start;                  
        remain = BLKSIZE - start;

        /*
        // Old method
        while(remain > 0){                      // copy bytes from buf to kbuf
            *cp++ = *buf++;
            poft -> offset++;
            if (poft -> offset > INODE.i_size){
                mip -> INODE.i_size++;
            }
            count++;
            remain--;
            nbytes--;
            if (nbytes <= 0){
                break;
            }

            //if (offset > fileSize)
        }   // end while(remain)
        */

       //New method
       if (remain <= nbytes){   
            memcpy(cp, cq, remain);
            cq += remain;
            cp += remain;
            poft ->offset += remain;
            nbytes -= remain;
       } else {
            memcpy(cp, cq, nbytes);
            cq += nbytes;
            cp += nbytes;
            poft ->offset += nbytes;
            nbytes -= nbytes;
       }
        if(poft -> offset > mip->INODE.i_size){
            mip -> INODE.i_size = poft -> offset;
        }
        put_block(mip->dev, blk, wbuf);
    }   // end while(byte)

    mip -> dirty = 1;
    //printf("Wrote %d char into file descriptor fs=%d\n", nbytes, fd);
    return nbytes;
}

int my_cp(char source[], char dest[]){
    //printf("In my_cp\n");
    char buf[BLKSIZE], tbuf[BLKSIZE];
    int fd, gd, temp;

    //printf("Before string copy\n");
    strcpy(tbuf, source);
    //strcpy(source, tbuf);
    //printf("%s", tbuf);
    dest[strlen(dest)] = 0;
    fd = open_file(source, "0");
    gd = open_file(dest, "1");

    //printf("fd = %d, gd = %d", fd, gd);

    while(temp = read_file(fd, buf, BLKSIZE)){
        //printf("In while loop\n");
        //printf("%d", temp);
        mywrite(gd, buf, temp);
    }

    close_file(fd);
    close_file(gd);

}
