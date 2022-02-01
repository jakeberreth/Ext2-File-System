//read_cat

int read_file(int fd, char* buf, int nbytes) {

    OFT *oft = running->fd[fd];
    if(oft == 0)
    {
        printf("\n\nead_file() oft was NULL\n\n");
        return;
    }
    MINODE *mip = oft->mptr;
    if(mip == 0)
    {
        printf("\n\nread_file() minode was NULL\n\n");
        return;
    }
    INODE *inode = &mip->INODE;
    if(inode == 0)
    {
        printf("\nread_file() INODE was NULL");
        return;
    }
    //printf("\nread_file() structures OKAY\n");

    // int avil = sizeof(running->fd[fd]->mptr->INODE.i_size) - offset;
    int avil = inode->i_size - oft->offset;
    int count = 0, remain = 0, lbk = 0, start = 0, blk = 0;
    char* cq = buf;
    char *cp;

    char readBuf[BLKSIZE];
    // to point to readbuf as INT
    int *bptr;

    int n = 0;
    while(nbytes && avil) 
    {

        lbk = oft->offset / BLKSIZE;
        start = oft->offset % BLKSIZE;

        int doubleIndex = 0;
        int doubleOffset = 0;
        char doubleIBuf[BLKSIZE];
        int doubleBLK = 0;
        //printf("start=%d, lbk=%d\n", start, lbk);

	    //getchar();
	    //printf("lbk=%d\n", lbk);
        //offset is not retaining changes for each loop


	    if(lbk < 12) //DIRECT
        {
            //printf("direct block\n");
            blk = inode->i_block[lbk];
        }
	    else if(lbk >= 12 && lbk < 12 + 256)  //INDIRECT
        {
            //printf("indirect block\n");
            get_block(dev, inode->i_block[12], readBuf);
            bptr = (int*)readBuf;
            blk = bptr[lbk - 12]; //get spot within indirect by subtracting thta beginning 12 from lbk
        } 
        else // DOUBLEINDIRECT
        {
            //printf("double-indirect block\n");
            // get DI blocks as chars
            get_block(mip->dev, inode->i_block[13], readBuf);

            // mailmans alg
            doubleIndex = (lbk - 12 - 256) / 256;
            doubleOffset = (lbk - 12 - 256) % 256;

            // cast read buf to int
            bptr = (int *)readBuf;

            // get indirect blocks
            get_block(mip->dev, bptr[doubleIndex], readBuf);

            // cast
            bptr = (int *)readBuf;

            // get Indirect block number from doubleindirect offset
            blk = bptr[doubleOffset];

	    }

        char finalBuf[BLKSIZE];
        bzero(finalBuf, 1024);

        get_block(dev, blk, finalBuf);

        cp = finalBuf + start;
        remain = BLKSIZE - start;

	    if(nbytes < remain) 
        {
            remain = nbytes;
        }
        if(avil < remain) 
        {
	        remain = avil;
        }
	
        memcpy(cq, cp, remain);
        //printf("after mem\n");
        cq += remain;
        cp += remain;
        count+=remain;
        oft->offset += remain;
        avil -= remain;
        nbytes -= remain;
        remain = 0;
        //printf("a\n");     
        //cp does not hold the full readbuf string
        //printf("cq=%s, cp=%s, count=%d, offset=%d, avil=%d, nbytes=%d, remain=%d\n", cq, cp, count, running->fd[fd]->offset, avil, nbytes, remain);   
    }
    //printf("before return\n");
    return count;
}

int cat_file(char* filename) {
    //printf("in cat\n");
    char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
    int n, i = 0;

    printf("\n");

    int fd = open_file(filename, "0");
    int cont = 1;

    while(cont)
    {
        bzero(mybuf, 1024);
        n = read_file(fd, mybuf, 1024);
        mybuf[1024] = 0;
        printf("%s", mybuf);
        if(n == 0)
        {
            cont = 0;
        }
    }
    printf("\n");

     close_file(fd);
}

int map(INODE l, int lbk) {
    char ibuf[256];
    int blk = 0;
    if(lbk < 12) 
        blk = l.i_block[lbk];
    else if (12 <= lbk < 12 + 256) {
        put_block(dev, l.i_block[12], ibuf);
        blk = ibuf[lbk-12];
    }
    else {
        //nothing
    }
    return blk;
}
