/////////////////////////////////////////////////////
// Programers: Garth Bates, Jacob Berreth
// Class: CptS 360
// Project: Mount_root
// File: util.c
// Date Completed: 
///////////////////////////////////////////////////// 

int symlink(char* oldFile, char* newFile) {
    int oino = getino(oldFile, dev);
    MINODE* omip = iget(dev, oino);
    int ino;
    char s[100];
    sprintf(s, "%4x", omip->INODE.i_mode);
    if(s[0] == '4') {
        printf("Is a DIR\n");
        return;
    } else {
        ino = getino(newFile, dev);
        if(ino != 0) {
            printf("newFile already exists");
            return;
        } else {
            //printf("newFile does not exist\n");
            fileType = 2;
            creat_file(newFile);
            //printf("after creat_file\n");
            int nino = getino(newFile, dev);
            MINODE* nmip = iget(dev, nino);
            //printf("i_mode=%4x\n", nmip->INODE.i_mode);
            INODE* newInode = &nmip->ino;
            newInode->i_size = strlen(oldFile);
            //printf("%d %d\n", strlen(oldFile), nmip->INODE.i_size);
            //nmip->INODE.i_mode = 0x2FFF;
            nmip->INODE.i_mode = 012000;                       
            nmip->dirty = 1;
            for(int i = 0; i < 12; i++) {
                newInode->i_block[i] = 0;
            }
            
            //printf("%s %s\n", oldFile, linkFile);
            memcpy(&(nmip->INODE.i_block[0]), oldFile, strlen(oldFile));
            //printf("value %s\n", &(nmip->INODE.i_block[0]));
            //INODE* n = &nmip->ino;
           // printf("new value %s\n", &(n->i_block[0]));
            iput(nmip);
            //getchar();
            // char* parent = dirname(newFile);
            // int pino = getino(parent, dev);
            // MINODE* pmip = iget(dev, pino);
            // pmip->dirty = 1;
            // iput(pmip);
        }
    }
}