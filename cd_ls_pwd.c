/************* cd_ls_pwd.c file **************/

#include <time.h>
#include <string.h>
#include <ctype.h>

//2D binary to hex array conversion table
char* cTable[16][2] = {
  {"0", "0000"},
  {"1", "0001"},
  {"2", "0010"},
  {"3", "0011"},
  {"4", "0100"},
  {"5", "0101"},
  {"6", "0110"},
  {"7", "0111"},
  {"8", "1000"},
  {"9", "1001"},
  {"a", "1010"},
  {"b", "1011"},
  {"c", "1100"},
  {"d", "1101"},
  {"e", "1110"},
  {"f", "1111"}
};

int chdir(char *pathname)   
{
  //printf("chdir %s\n", pathname);
  
  //find mip with pathname
  int ino = getino(pathname, dev);
  MINODE *mip = iget(dev, ino);

  //set mode to i_mode (4 hex digits)
  unsigned int mode = mip->INODE.i_mode;

  //read digit
  char s[100];
  sprintf(s, "%4x", mip->INODE.i_mode);
  
  if(s[0] == '4') {
    //printf("%s is a directory.\n", pathname);
    iput(running->cwd);
    running->cwd = mip;
    //printf("%d\n", running->cwd->INODE.i_block[0]);
  } else {
    printf("%s is not a directory.\n", pathname);
    return;
  }
}

int ls_file(MINODE *mip, char *name)
{
  char *t1 = "rwxrwxrwx       ";

  //convert to binary
  char s[100];
  sprintf(s, "%4x", mip->INODE.i_mode);

  char c;
  char* nibbles[4];
  char newStr[2];
  int a = 0;
  char bTable[17] = "";

  //printf("%s\n", s);
  for(a = 0; a < 4; a++) {
    c = s[a];
    int val;
    char newStr[2] = {'0', '0'};
    newStr[0] = c;
    newStr[1] = '\0';
    //printf("%s\n", newStr);
    val = (int) newStr[0];
    //printf("%d \n", val);
    for(int i = 0; i < 16; i++) {
      char* st = cTable[i][0];
      if(st[0] == val) {
        nibbles[a] = cTable[i][1];
        //printf("%s\n", nibbles[a]);
        break;
      }
    }
    strcat(bTable, nibbles[a]);
  }

  //printf("%s\n", bTable);
  if(bTable[0] == '1')
    printf("-");
  if(bTable[0] == '0')
    printf("d");
  for(int i = 7; i < 16; i++) {
    if(bTable[i] == '1')
      printf("%c", t1[i-7]);
    else
    printf("-");
  }
  printf(" ");
  printf("%d ", mip->INODE.i_links_count);
  printf("%d ", mip->ino);
  printf("%4x ", mip->INODE.i_mode);
  printf("%d %d ", mip->INODE.i_uid, mip->INODE.i_gid);
  printf("%d ", mip->INODE.i_size);
  printf("%d ", mip->INODE.i_blocks);
  printf("%d ", mip->INODE.i_block[0]);
  time_t myTime = mip->INODE.i_mtime;
  char *p = (char *)ctime(&myTime);
  p[strlen(p) - 1] = '\0';
  printf("%s ", p);
  if(s[0] != '1')
   printf("%s \n", name);
  else {
    printf("%s -> %s\n", name, &(mip->INODE.i_block[0]));
  }
}

int ls_dir(MINODE *mip)
{
  //printf("%d %d \n", mip->dev, mip->ino);

  printf("\n");
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  
  // Assume DIR has only one data block i_block[0]

  //check i_block[0]
  //printf("%d \n", mip->INODE.i_block[0]);
  
  get_block(dev, mip->INODE.i_block[0], buf); 
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     //printf("[%d %s]  ", dp->inode, temp); // print [inode# name]

     MINODE *mip = iget(dev, dp->inode);
     ls_file(mip, temp);

     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");
}

int ls(char *pathname)  
{ 
  //printf("pathname=%s\n", pathname);
  if(pathname[0] == '\0') {
    ls_dir(running->cwd);
  } else {
    int ino = getino(pathname, dev);
    MINODE *mip = iget(dev, ino);
    ls_dir(mip);
  }
  
}

char *rpwd(MINODE *wd){
  // 1
  if (wd->ino == 2){
    //printf("is root\n");
    //getchar();
    return;
  }

  // 2
  unsigned int myINODE = 0;
  int parent_ino = findino(wd, &myINODE);
  char buf[1024];
  //printf("parent num=%d\n", parent_ino);

  char my_name[256];
  bzero(my_name, 256);

  // 3
  MINODE *pip = iget(dev, parent_ino);
  //printf("%d\n", pip->INODE.i_block[0]);
  //printf("%d %d\n", pip->dev, pip->ino);

  findmyname(pip, myINODE, my_name);
  rpwd(pip);

  printf("/");

  printf("%s", my_name);

  iput(pip);
}

char *pwd(MINODE *wd)
{
  printf("\n");
  if (wd == root){
    printf("/\n");
    return;
  } else{
    rpwd(wd);
    printf("\n");
  }

}



