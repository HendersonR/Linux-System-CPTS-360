#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <ext2fs/ext2_fs.h>

#include "tools.c"




int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}
int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);//get imap from memory

  for (i=0; i < ninodes; i++){//while there are still inodes 
    if (tst_bit(buf, i)==0){//if the inode isn't being used'
       set_bit(buf,i);//set it to 1
       decFreeInodes(dev);//decrease free inodes

       put_block(dev, imap, buf);//write imap back to memory

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}
int idealloc(int dev, unsigned long ino)
{
  char buf[BLKSIZE];
  get_block(dev,imap,buf);//get imap
  clr_bit(buf,ino-1);//set clear bit to 0
  incFreeInodes(dev);//increase free inode count
  put_block(dev,imap,buf);//write imap back to memory

}
/*
char *disk = "mydisk";

main(int argc, char *argv[ ])
{
  int i, ino;
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

  imap = gp->bg_inode_bitmap;
  printf("imap = %d\n", imap);
  getchar();

  for (i=0; i < 5; i++){  
    ino = ialloc(fd);
    printf("allocated ino = %d\n", ino);
  }
 
}*/
