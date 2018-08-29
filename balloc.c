#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <ext2fs/ext2_fs.h>

#include "ialloc.c"


int balloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap, buf);

  for (i=0; i < ninodes; i++){//while there are still inodes available
    if (tst_bit(buf, i)==0){//if that bit in the bitmap is a 0..
       set_bit(buf,i);//set it to a 1
       decFreeBlocks(dev);//decreace amount of free block

       put_block(dev, bmap, buf);//write bmap back to memory

       return i+1;
    }
  }
  printf("balloc(): no more free blocks\n");
  return 0;
}

int bdealloc(int dev, unsigned long block)
{
  char buf[BLKSIZE];
  get_block(dev,bmap,buf);//get bmap
  clr_bit(buf,block-1);//set its bit to 0
  incFreeBlocks(dev);//increase amount of free blocks
  put_block(dev,bmap,buf);//write bmap back to memory

}
/*
char *disk = "mydisk";

main(int argc, char *argv[ ])
{
  int i, bno;
  char buf[BLKSIZE];

  if (argc > 1)
    disk = argv[1];

  fd = open(disk, O_RDWR);
  if (fd < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  nfreeInodes = sp->s_free_inodes_count;
  nfreeBlocks = sp->s_free_blocks_count;
  printf("ninodes=%d nblocks=%d nfreeInodes=%d nfreeBlocks=%d\n", 
	 ninodes, nblocks, nfreeInodes, nfreeBlocks);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  printf("bmap = %d\n", bmap);
  getchar();

  for (i=0; i < 5; i++){  
    bno = balloc(fd);
    printf("allocated bno = %d\n", bno);
  }
 
}*/
