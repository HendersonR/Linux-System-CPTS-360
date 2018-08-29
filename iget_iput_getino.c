#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "type.h"
#include "balloc.c"
//#include "tools.c"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

/*
int dev;
int nblocks;
int ninodes;
int bmap;
int imap;
int iblock;*/

char buf[BLKSIZE], dbuf[BLKSIZE], sbuf[256], ibuf[BLKSIZE];

int fd;
int block, offset;//iblock, disp
int iblock, rootblock;
int INODES_PER_BLOCK=BLKSIZE/sizeof(INODE);

//char *dev = "mydisk";     //default device
//char *disk = "mydisk";

/* ************Functions As Before**************/
/*
int get_block(int fd, int blk, char *buf)
{
    lseek(fd, blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}*/
/* ************Functions As Before END**************/



/****************** Search Code *********************/
int searchM(MINODE *p, char *name)
{
    int inode_number=0; 
    inode_number=search(p->INODE,name);
    return inode_number;       
} 
/****************** Search Code END*********************/
/****************** iget Code **************************/
MINODE *iget(int dev, int ino)
{
  int i, blk, disp;
  char buf[BLKSIZE];
  MINODE *mip;
  INODE *ip;
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       printf("found [dev:%d ino:%d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       printf("allocating NEW minode[%d] for [dev:%d ino:%d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev; mip->ino = ino;  // assing to (dev, ino)
       mip->dirty = mip->mounted = mip->mptr = 0;
       // get INODE of ino into buf[ ]      
       blk  = (ino-1)/8 + iblock;  // iblock = Inodes start block #
       disp = (ino-1) % 8;
       //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);
       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}
/****************** iget Code END**************************/

/****************** iput Code *****************************/
int iput(MINODE *mip)  // dispose of a minode[] pointed by mip
{
    int block, disp;
    //1
    mip->refCount--;
    //2
    if (mip->refCount > 0) return;
    if (!mip->dirty)       return;
    /*3 write INODE back to disk */
    printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino); 
    //Use mip->ino to compute: blk containing this INODE,disp of INODE in block
    block = (mip->ino-1)/INODES_PER_BLOCK + iblock;
    disp = (mip->ino-1)%INODES_PER_BLOCK;
           
     get_block(mip->dev, block, buf);

     ip = (INODE *)buf + disp;
     *ip = mip->INODE;
     put_block(mip->dev, block,buf);
}
/****************** iput Code END *************************/

/****************** getino CODE ***************************/

int getino(int *dev, char *pathname)
{

  int i, ino, blk, disp;
  //added by me 
  int n=0;
  char *word;
  char hcrname[256][256];
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;
  
  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
  {
      return 2;
  }
  if (pathname[0]=='/')
  {
     mip = iget(dev, 2);
  }
  else
  {
    //search(running->cwd->ino,)
     mip = iget(running->cwd->dev, running->cwd->ino);
  }
    
  strcpy(buf, pathname);
  //tokenize(buf); // n = number of token strings
  word = strtok(buf,"/");
   while(word != NULL)
    {
        printf("word[%d]: %s %d\n", n, word,n);
        if(word!=NULL)
        {
            strcpy(hcrname[n],word);
        }
        word = strtok(NULL,"/");
        
        n++; // n = number of token strings
    }
  
  for (i=0; i < n; i++)
  {
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, hcrname[i]);
 
      ino = search(mip, hcrname[i]);
      //printf("out of search\n");
      //?
      /*if (ino==0)
      {
         iput(mip);
         printf("name %s does not exist\n", hcrname[i]);
         return 0;
      }*/
      printf("name: %s  ino:%d\n",hcrname[i],ino);
      iput(mip);
      
      mip = iget(dev, ino);
  }
  iput(mip);
  //printf("still in getino\n");
  return ino;
}
/*******************getino Code END ************************/



/********** Added from showblock.c ****************/

/* *note the added code is modified */

/********************Showblock.c Code *************************/

void s(char *diskImage, char *Pathname)
{
    /*char buf[BLKSIZE], dbuf[BLKSIZE], sbuf[256];
    int fd;
    int iblock, rootblock;
    char *dev = "mydisk"; */

    //int fd;
    int i=0;
    int n = 0;
    int inumber=0;
    int check = 0;
    //int rootblock;
    int InodesBeginBlock = 0;
    char *word;
    char *cp;
    char path[256];
    char paths[256][256];
    //const char *word[256];

   //file should be open by now

    //Read in Superblock (blk #1 for FD to verify it's indeed an ext2 FS.)
    get_block(fd, 1, buf);
    sp = (SUPER *)buf;
    printf("FD after super: %d\n");
    printf("check ext2 FS on %s ", dev);
    if (sp->s_magic != 0xEF53){
       printf("NOT an EXT2 FS\n");
       exit(1);
    }
    else
    {
        printf("YES\n");
    }
    printf("**************basic info******************\n");
    //nblocks, ninodes, ngroups, inodes_per_group, number of free inodes and blocks
    printf("nblocks: %d\t",sp->s_blocks_count);
    printf("ninodes: %d\n",sp->s_inodes_count);
   //printf("ngroups: %d\t",sp->);
    printf("inodes per group: %d\n",sp->s_inodes_per_group);
    printf("free inodes: %d\t",sp->s_free_inodes_count);
    printf("free blocks: %d\n",sp->s_free_blocks_count);
    printf("*********************************************\n");

    //Read in group descriptor block
    get_block(fd, 2, buf);  
    gp = (GD *)buf;
    InodesBeginBlock=gp->bg_inode_table;
    printf("FD after gd: %d\n",fd);

    //read first INODE block to get root inode #2
    get_block(fd, InodesBeginBlock, buf);
    ip =  (INODE *)buf + 1;
    printf("FD first inode: %d\n",fd);

    printf("******* root inode info ********\n");
    printf("mode=%4x ", ip->i_mode);
    printf("uid=%d gid=%d\n", ip->i_uid, ip->i_gid);
    printf("size=%d\n", ip->i_size);
    printf("time=%s", ctime(&ip->i_ctime));
    printf("link=%d\n", ip->i_links_count);
    printf("i_block[0]=%d\n",ip->i_block[0]);
    rootblock=ip->i_block[0];
    printf("*******************************\n");

    /*redundant
    //consider i_block[0] ONLY
    get_block(fd, ip->i_block[0], dbuf);
    dp = (DIR *)dbuf;
    cp = dbuf;
    printf("FD after iblock[0]: %d\n",fd);*/

    //tokenize pathname
    strcpy(path,Pathname);
    word=strtok(path,"/");
    while(word != NULL)
    {
        
       
        if(word!=NULL)
        {
            strcpy(paths[n],word);
        }
        //printf("word %d: %s\n",n,paths[n]);
        word = strtok(NULL,"/");
        n++;
    }
    printf("n: %d\n",n);

    //look for pathname
    //block = (inumber-1)/INODES_PER_BLOCK + InodesBeginBlock;
    //get_block(fd,block,dbuf);
    block=rootblock;
    printf("FD before loop: %d\n",fd);
    while(i<n)
    {
        //printf("starting search\n");
        if(S_ISDIR(ip->i_mode)!=0)//ip is a directory
        {
            inumber = search(ip,paths[i]);
        }
        else
        {
            printf("not a directory\n");
            break;
        }
        //printf("number: %d\n",inumber);
        if(inumber!=0)//name was found, ino found 
        {
            printf("%s found\n",paths[i]);
            block = (inumber-1)/INODES_PER_BLOCK + InodesBeginBlock;
            offset = (inumber-1)%INODES_PER_BLOCK;
            //get_block(fd,Inode,buf);
            ip = (INODE *)ibuf + offset;
            printf("Block:%d\t Inode:%d\n",block,offset);

        }
        else//name not found, ino not found
        {
            printf("name '%s' wasn't found\n",paths[i]);
            break;
        }
        printf("i: %d\n",i);
        i++;
    }
    
}

//search function
int search(INODE *ip, char *name)
{
    
    char* cp;
    int inode_number = 0;
    unsigned long *indirect;
    unsigned long *doubleIndirect;
    int indblk;
    int dbindblk;
    int j=0;
    int i=0;
    int r=0;
    int z=0;
    int flag=0;//turns to 1 if there was an issue finding the name
    int check = 1;
    char sbuf[256];

    //printf("in search\n");
    while(i<14)//i:0-11 direct i:12 indirect i:13 double indirect
    {
        if(i<12)//direct
        {
            //search for name in current iblock
            get_block(fd, ip->i_block[i], ibuf);
            dp = (DIR *)ibuf;
            cp = ibuf;
            while(cp < &ibuf[BLKSIZE])
            {
                strncpy(sbuf, dp->name, dp->name_len);
                sbuf[dp->name_len] = 0;
                check = strcmp(sbuf,name);
                if(check==0)//found name
                {
                    printf("name found in i_block[%d]\n",i);
                    inode_number = dp->inode;
                    printf("inode_number: %d\n",inode_number);
                    break;
                }
                cp += dp->rec_len;
                dp = (DIR *)cp;
                if(dp->rec_len==0)//inside of empty block, havent found name  
                {
                    flag=1;//set issue flag
                    break;//exit search

                }
                //printf("cp: %d\n",cp);
                //printf("dp->reclen: %d\n",dp->rec_len);
            }
        }
        if(i==12)//indirect
        {
            //search for name in subblock of iblock[12]
            /*r=0;
            while(r<256)
            {
                get_block(fd,ip->i_block[12]->i_block[r],ibuf);
                dp = (DIR *)ibuf;
                cp = ibuf;
                while(cp < &ibuf[BLKSIZE])
                {
                    strncpy(sbuf, dp->name, dp->name_len);
                    sbuf[dp->name_len] = 0;
                    check = strcmp(sbuf,name);
                    if(check==0)//found name
                    {
                        inode_number = dp->inode;
                        break;
                    }
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }
                r++;
                if(check==0)//if name was found
                {
                    break;
                }
            }*/

        }
        if(i==13)//double indirect
        {/*
            //search for name in subblock of subblock of iblock[13]
             r=0;
            while(r<256)
            {
                while(z<256)
                {
                    get_block(fd,ip->i_block[12].iblock[r].iblock[z],ibuf);
                    dp = (DIR *)ibuf;
                    cp = ibuf;
                    while(cp < &ibuf[BLKSIZE])
                    {
                        strncpy(sbuf, dp->name, dp->name_len);
                        sbuf[dp->name_len] = 0;
                        check = strcmp(sbuf,name);
                        if(check==0)//found name
                        {
                            inode_number = dp->inode;
                            break;
                        }
                        cp += dp->rec_len;
                        dp = (DIR *)cp;
                    }
                    z++;
                    if(check==0)//name was found
                    {
                        break;
                    }
                }
                r++;
                if(check==0)//name was found
                {
                    break;
                }
               
            }*/
        } 
        if(flag==1);//there was an issue (name may not exist in expected location)
        {
            break;//stop searcing
        } 
        i++;
        if(check==0)//if name was found and number was retrieved
        {
            break;//stop
        }
    }
        return inode_number;
} 
