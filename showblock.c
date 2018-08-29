#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

char buf[BLKSIZE], dbuf[BLKSIZE], sbuf[256], ibuf[BLKSIZE];

int fd;
int block, offset;
int iblock, rootblock;
int INODES_PER_BLOCK=BLKSIZE/sizeof(INODE);

char *dev = "mydisk";     //default device

int get_block(int fd, int blk, char *buf)
{
    lseek(fd, blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

char *disk = "mydisk";

/*************** DUE & DEMO : Week of 3-27-2017 ********************
 
 Write a C program, showblock, which displays the disk blocks of a file 
 in an EXT2 file system. The program runs as follows

       showblock  DEVICE    PATHNAME
       ---------  -------   ----------
e.g.   showblock  diskimage  /a/b/c/d  (diskimage contains an EXT2 FS)
OR     showblock  /dev/sda2  /x/y/z    (/dev/sda2 is YOUR HD partition)

 It locates the file named PATHNAME and prints the disk blocks (direct, 
 indirect, double-indirect) of the file.
**************************************************************************/ 

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

    //Open device for read only
    fd = open(disk, O_RDONLY);
    if (fd < 0){
    printf("open failed\n");
    exit(1);
    }
    printf("FD: %d\n",fd);
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
    //printf("word: %s\n",word);
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
    //problem?
    char* cp;
    int inode_number = 0;
    int check = 1;
    char sbuf[256];
    get_block(fd, ip->i_block[0], ibuf);
    dp = (DIR *)ibuf;
    cp = ibuf;
    printf("cp: %d\n",cp);
    printf("&ibuf[BLKSIZE]: %d\n",&ibuf[BLKSIZE]);

    while(cp < &ibuf[BLKSIZE]){
        
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;
        
        check = strcmp(sbuf,name);
        printf("check: %d\n",check);
        if(check==0)//found name
        {
            inode_number = dp->inode;
            break;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
        printf("cp: %d\n",cp);
        printf("&ibuf[BLKSIZE]: %d\n",&ibuf[BLKSIZE]);
        }
        printf("done\n");
        return inode_number;
} 

main(int argc, char *argv[ ])
{
    s("Disk_Image","/a/b");
    
}
