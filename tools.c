#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <ext2fs/ext2_fs.h>

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

#define BLKSIZE 1024

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

/********** globals *************/
int fd;
int imap, bmap;  // IMAP and BMAP block number
int ninodes, nblocks, nfreeInodes, nfreeBlocks;

int get_block(int fd, int blk, char buf[ ])
{
  //printf("get_block: fd=%d blk=%d buf=%x\n", fd, blk, buf);
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}
int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);

}
/*int put_block(int dev, int blk,char buf[])
{
  lseek(dev,BLKSIZE*blk,SEEK_SET);
  write(dev,buf, BLKSIZE);
  return;
}*/

int tst_bit(char *buf, int bit)//test if a bit is one or zero
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}
int set_bit(char *buf, int bit)//set a bit to 1
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)//clar a bit to 0 
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}
int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free blocks count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}
int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free blocks count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}
