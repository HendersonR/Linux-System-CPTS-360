#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR; 

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

#define BLKSIZE 1024

char buf[BLKSIZE], dbuf[BLKSIZE], sbuf[256];

int fd;
int iblock, rootblock;

char *dev = "mydisk";     //default device

int get_block(int fd, int blk, char *buf)
{
    printf("get_block: fd=%d blk=%d buf=%x\n", fd, blk, buf);
    lseek(fd, blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int dir(char *dev)
{
    int i; char *cp;
    char* name = "L";
    int ran=0;

    fd = open(dev, O_RDONLY);
    if (fd < 0){
        printf("open %s failed\n", dev);
        exit(1);
    }

    /***************
    // read SUPER block at offset 1024
    *****************/

    get_block(fd, 1, buf);
    sp = (SUPER *)buf;

    printf("check ext2 FS on %s ", dev);
    if (sp->s_magic != 0xEF53){
       printf("NOT an EXT2 FS\n");
       exit(1);
    }
    else
    {
        printf("YES\n");
    }

    //read GD block at (first_data_block + 1)
    get_block(fd, 2, buf);
    gp = (GD *)buf;

    printf("GD info: %d %d %d %d %d %d\n",
            gp->bg_block_bitmap,
            gp->bg_inode_bitmap,
            gp->bg_inode_table,
            gp->bg_free_blocks_count,
            gp->bg_free_inodes_count,
            gp->bg_used_dirs_count);

    iblock = gp->bg_inode_table;
    printf("inodes begin block=%d\n",iblock);

    //read first INODE block to get root inode #2
    get_block(fd, iblock, buf);
    ip =  (INODE *)buf + 1;

    printf("******* root inode info ********\n");
    printf("mode=%4x ", ip->i_mode);
    printf("uid=%d gid=%d\n", ip->i_uid, ip->i_gid);
    printf("size=%d\n", ip->i_size);
    printf("time=%s", ctime(&ip->i_ctime));
    printf("link=%d\n", ip->i_links_count);
    printf("i_block[0]=%d\n",ip->i_block[0]);
    rootblock=ip->i_block[0];
    printf("*******************************\n");

    //consider i_block[0] ONLY
    get_block(fd, ip->i_block[0], dbuf);
    printf("dbuf: %s\n",dbuf);
    dp = (DIR *)dbuf;
    cp = dbuf;

    while(cp < &dbuf[BLKSIZE]){
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;
        printf("%4d %4d %4d %s\n", 
                dp->inode, dp->rec_len, dp->name_len, sbuf);
        cp += dp->rec_len;
        dp = (DIR *)cp;


        /****************************************
        unsigned long inode
                unsigned short rec_len;    //This entry length in bytes
                unsigned char name_len;    //Name length in bytes
                unsigned char file_type;   // for future use
                char name[EXT2_NAME_LEN];
        *****************************************/        
    }
    //ran = search(ip,name);
    //printf("inode number: %d\n",ran);
}
//search function
int search(INODE *ip, char *name)
{
    char* cp;
    int inode_number = 0;
    int check = 1;
    char sbuf[256];
    get_block(fd, ip->i_block[0], dbuf);
    dp = (DIR *)dbuf;
    cp = dbuf;

    while(cp < &dbuf[BLKSIZE]){
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;
        printf("sbuf: %s\n",sbuf);
        printf("name: %s\n",name);
        check = strcmp(sbuf,name);
        printf("check: %d\n",check);
        if(check==0)//found name
        {
            inode_number = dp->inode;
            break;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
        }
        printf("done\n");
        return inode_number;
}
char *disk = "mydisk";

main(int argc, char *argv[ ])

{ 
    char* name ="a";
  if (argc > 1)
    disk = argv[1];
  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open failed\n");
    exit(1);
  }
  dir(disk);
  

  
}
