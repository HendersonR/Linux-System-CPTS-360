#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <ext2fs/ext2_fs.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

#define BLKSIZE 1024

/*********************************************
struct ext2_group_desc
{
    u32 bg_block_bitmap;       //Bmap block number
    u32 bg_inode_bitmap;       //Imap block number
    u32 bg_inode_table;        //Inodes begin block number
    u16 bg_free_blocks_count;  //THESE are OBVIOUS
    u16 bg_free_inodes_count;
    u16 bg_used_dirs_count;
    u16 bg_pad;                //ignore these
    u32 bg_reserved[3];
};
**********************************************/

char buf[BLKSIZE];
int fd;

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}

gd()
{
    //read 
    get_block(fd, 2, buf);  
    gp = (GD *)buf;

    //print contents
    printf("********* group 0 information ************\n");
    printf("Bmap block number: %d\n",gp->bg_block_bitmap);
    printf("Imap block number: %d\n",gp->bg_inode_bitmap);
    printf("Inodes begin block number: %d\n",gp->bg_inode_table);

    printf("Free block count: %d\n",gp->bg_free_blocks_count);
    printf("Free inode count: %d\n",gp->bg_free_inodes_count);
    printf("Used directories count: %d\n",gp->bg_used_dirs_count);



}

char *disk ="mydisk";

main(int argc, char *argv[ ])

{ 
  if (argc > 1)
    disk = argv[1];
  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open failed\n");
    exit(1);
  }

  gd();
}
