
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h> 

#include "type.h"

// global variables

MINODE minode[NMINODE];        // global minode[ ] array           
MINODE *root;                  // root pointer: the /    
PROC   proc[NPROC], *running;  // PROC; using only proc[0]

int fd, dev;                               // file descriptor or dev
int nblocks, ninodes, bmap, imap, iblock;  // FS constants

#include "iget_iput_getino.c"  // YOUR iget_iput_getino.c file with
                               // get_block/put_block, tst/set/clr bit functions
 
char *disk = "mydisk";//mydisk
char line[128], cmd[64], pathname[64], path2[64];
char *temp;
char buf[BLKSIZE];              // define buf1[ ], buf2[ ], etc. as you need
char buf1[BLKSIZE];
char bufls[BLKSIZE];
char buf2[BLKSIZE];
char bufrmc[BLKSIZE];

main(int argc, char *argv[ ])   // run as a.out [diskname]
{
  if (argc > 1)
     disk = argv[1];

  if ((dev = fd = open(disk, O_RDWR)) < 0){
     printf("open %s failed\n", disk);  
     exit(1);
  }
  //print fd or dev to see its value!!!
  printf("fd: %d dev:%d\n",fd,dev);

  //super() (code from super.c)
  printf("checking EXT2 FS\n");
  //read super block
   get_block(fd, 1, buf);
   sp = (SUPER *)buf;

   //Write C code to check EXT2 FS; if not EXT2 FS: exit
   //check for magic number 
   printf("s_magic = %x\n", sp->s_magic);
   if (sp->s_magic != 0xEF53)
   {
     printf("NOT an EXT2 FS\n");
     exit(1);
   }
   printf("EXT2 FS OK\n");
   
   //get ninodes, nblocks (and print their values)
   ninodes = sp->s_inodes_count;
   nblocks = sp->s_blocks_count;
   printf("ninodes: %d\t nblocks: %d\n",ninodes,nblocks);
   
   //Read GD block to get bmap, imap, iblock (and print their values)
   //read 
    get_block(fd, 2, buf);  
    gp = (GD *)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    iblock = gp->bg_inode_table;

    printf("bmap: %d\t imap:%d\t iblock: %d\n",bmap,imap,iblock);

    printf("processing\n\n");
    //initialize
    printf("running int...\n");
    init();

    //mount root
    printf("running mount_root...\n");
    mount_root();   // write C code

    //printf("creating P0 as running process\n");
    //WRITE C code to do these:     
    //set running pointer to point at proc[0];
    running = &proc[0];
    //
    //set running's cwd   to point at / in memory;
    running->cwd = iget(root->dev,root->ino);

    // command processing loop
    while(1)     
    {
       printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|stat|chmod|menu|touch|quit] ");
       //WRITE C code to do these:
       
       //-use fgets() to get user inputs into line[128]
       fgets(line,128,stdin);
       //kill the \r at end 
       line[strlen(line)-1]=0;
       //if (line is an empty string) // if user entered only \r
       if(line[0]=='\r')
       {
         //continue;
         continue;
       }
        
       //Use sscanf() to extract cmd[ ] and pathname[] from line[128]
       
       memset(pathname,0,strlen(pathname));//clear pathname 
       memset(cmd,0,strlen(cmd));//clear cmd
       memset(path2,0,strlen(path2));//clear path2
     
       sscanf(line,"%s %s %s",cmd,pathname,path2);
      
       //printf("cmd=%s pathname=%s\n", cmd, pathname);
       printf("cmd=%s pathname=%s\n", cmd, pathname);
       printf("path2=%s\n",path2);
       

       // execute the cmd
       if (strcmp(cmd, "ls")==0)//ls
       {
          ls(pathname);
       }
       if (strcmp(cmd, "cd")==0)//change directory
       {
          chdir(pathname);
       }
       if (strcmp(cmd, "pwd")==0)//print working directory
       {
          pwd(running->cwd);
       }
       
       if (strcmp(cmd, "mkdir")==0)//make directory
       {
          make_dir();
       }

       if (strcmp(cmd, "rmdir")==0)//remove direcory
       {
         rmdir();
       }
       
       if(strcmp(cmd, "creat")==0)//create
       {
          creat_file(pathname);
       }
       if(strcmp(cmd,"rm")==0)
       {
         rm();
       }
       if(strcmp(cmd,"link")==0)//link
       {
          link(pathname,path2);
       }
       if(strcmp(cmd,"unlink")==0)//unlink
       {
         unlink(pathname);
       }
       if(strcmp(cmd,"symlink")==0)//sysmlink
       {
         symlink(pathname,path2);
       }
       if(strcmp(cmd,"stat")==0)//stat
       {
         mystat();
       }
       if(strcmp(cmd,"chmod")==0)//chmod
       {
         mychmod();
       }
       if(strcmp(cmd, "touch")==0)//touch
       {
         mytouch();
       }
       if(strcmp(cmd,"menu")==0)//menu
       {
         menu();
       }
       if(strcmp(cmd,"readlink")==0)//readlink
       {
         readlink();
       }
       if (strcmp(cmd, "quit")==0)
       {
          quit();
       }

       //level 2
       if(strcmp(cmd,"open")==0)
       {
         //open_file(pathname,path2);
       }
       if(strcmp(cmd,"close")==0)
       {

       }
       if(strcmp(cmd,"pfd")==0)
       {
         //pfd();
       }
       
     }
}

int init()
{
   //WRITE C code to initialize
  MINODE *root = 0;
  int i=0;
  proc[0].uid=0;
  proc[0].cwd=0;
  proc[1].uid=1;
  proc[1].cwd=0;
  //All proc[]'s cwd=0
  while(i<NPROC)
  {
    proc[i].cwd=0;
    i++;
  }
  i=0;
  //All minode's refCount=0;
  while(i<NMINODE)
  {
    minode[i].refCount=0;
    i++;
  }
  i=0;
  //All fd of proc's 0 & 1 = 0
  while(i<NFD)
  {
    proc[0].fd[i]=0;
    proc[1].fd[i]=0;
    i++;
  }
  //return;
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);         // Do you understand this?
  printf("root->dev: %d\t root->ino:%d\n",root->dev,root->ino);
  //Explanation
  /* dev represents the begining of the open device, so passing in dev and 2 to iget makes a new 
  MINODE, then the INODE inside of the new MINODE is set to the root with the ino of 2 because the first 
  inode is the super*/
}

void menu()
{
  printf(".........................Commands.........................\n");
  printf("level-1\n");
  printf("ls...........................................................ls function\n");
  printf("cd [pathname]....................................changes cwd to pathname\n");
  printf("pwd ...........................................................print cwd\n");
  printf("mkdir [pathname]......................... make new directory at pathname\n");
  printf("rmdir [pathname]............................remove directory at pathname\n");
  printf("creat [pathname]....................... creat a new REG file at pathname\n");
  printf("rm [pathname]................................remove REG file at pathname\n");
  printf("link [oldpath][newpath].........creat a hard link to oldpath via newpath\n");
  printf("unlink [pathname].........................remove a hard link at pathname\n");
  printf("symlink [oldpath][newpath]......creat a soft link to oldpath via newpath\n");
  printf("readlink [pathname]......................print the contents of a symlink\n");
  printf("stat [pathname]................................prints status of pathname\n");
  printf("chmod [mode][pathname]..................changes the mode of the pathname\n");
  printf("touch [pathname].........creates a file or updates its modification time\n");
  printf("quit...................................................exits the program\n");
  printf("level-2\n");
  printf("open [pathname][mode]................open a REG file based on input mode\n");
  printf("                mode: 0:Read 1:Write 2:Read/Write 3:APPEND\n");
  printf("close [pathname]............................................closes a file\n");
  printf("pfd.........................................displays current opened files\n");
  printf("lseek [pathname].........................................................\n");
  printf("write [pathname]............................write text into opened file..\n");
  printf("read  [pathname]....................................read and display text\n");
  printf("cat [pathname].......................................show conents of file\n");
  printf("cp [file1] [file2]........................copy contents from file to file\n");
  printf("mv [file] [file2].......................................move file to file\n");

}
 
int ls(char *pathname)  // dig out YOUR OLD lab work for ls() code 
{
  MINODE *mip;
  MINODE *temp;
  DIR *dp;
  int ino=0;
  int dino=0;
  int indblk;
  int dbindblk;
  int i=0;
  int j=0;
  int n=0;
  int check=0;
  int pexist=0;//parent exist
  unsigned long *indirect;
  unsigned long *doubleIndirect;
  char *word;
  char *cp;
  char *parent;
  char *child;
  char holder[64];
  char path[256];
  char paths[256][256];
  
  //printf("inside\n");
  //printf("pathname[0]: %c\n",pathname[0]);
 

    //determine initial dev: 
    if(pathname[0]== '/')
    {
      dev = root->dev;
      printf("got dev from root\n");
    } 
    else
    {
      dev = running->cwd->dev;
      //printf("got dev from running\n");
    }  

    //check to see if there is a pathname  
    if(pathname[0]==0)//no pathname specified, print cwd
    {
      mip = iget(dev,running->cwd->ino);
      printf(":no pathname:\n");
    } 
    else //pathname specified
    {   
      printf(":pathname given:\n");
      memset(holder,0,64);
      strcpy(holder,pathname);
      strcat(holder,"/");        
      //convert pathname to (dev, ino);
      ino=getino(dev,pathname);
      //printf("ino: %d\n",ino);

      if(ino==0)//the specified pathname has a problem
      {
        return;//stop
      }
      //get a MINODE *mip pointing at a minode[ ] with (dev, ino);
      mip=iget(dev,ino);
    }
    //is a regular file
    if(S_ISREG(mip->INODE.i_mode)!=0)
    {
      printf("regular file\n");
      //ls_file(pathname);
      //check if the pathname has parents
      i=0;
      while(i<strlen(pathname))
      {
        if(pathname[i]=='/')//theres is at least 1 parent
        {
          pexist=1;
          break;//once parents existence is confirmed exit while to save time
        }
        i++;
      }
      if(pexist==1)//parents exist,get parent and child
      {
        parent = dirname(pathname);
        child = basename(holder);
        printf("ls: %s/%s\n",parent,child);
      }
      if(pexist==0)//no parents,get child
      {
        child = basename(holder);
        printf("ls: %s\n",child);
      }

    } //mip->INODE is a file: ls_file(pathname);
      
    //is a directory
    if(S_ISDIR(mip->INODE.i_mode)!=0)
    {
      printf("directory\n");
      /*is a DIR{step through DIR entries:for each name, ls_file(pathname/name);*/
      //printf("dev: %d\n",dev);
      //printf("fd: %d\n",fd);
      //cycle through direct blocks 
      //*note*i_block[0] is root block meaning dirs in this block are under root
      //while(i<11)
      //{//?
        //get_block(dev,mip->INODE.i_block[i],buf);//get the blocks of the inode in question
        //dp = (DIR *)buf;
        //cp = buf;
        //printf("&buf[BLKSIZE]: %d\n",&buf[BLKSIZE]);
        //printf("cp: %d\n",cp);
        //printf("dp->rec_len: %d\n",dp->rec_len);
        //getchar();
          //?
          //new
          i=0;
      while(i<12)
      {
          get_block(dev,mip->INODE.i_block[i],buf);//get first block
          cp = buf;
          dp = (DIR*) buf;
          check =0;
        while(cp<&buf[BLKSIZE])//while cp remains less than the size of the block being examined
        {
          //printf("inside\n");
          if(dp->rec_len==0)//empty block stop
          {
            break;
          }
          strncpy(bufls,dp->name,dp->name_len);//copy name into bufls 
          bufls[dp->name_len]=0;

          printf("%s\t",bufls);//print name 
          
          cp += dp->rec_len;//move cp further down the block in question by length of the (dir/file) it was just inspecting i.e rec_len
          dp = (DIR *) cp;//set dp to new cp location
          //printf("dp->rec_len: %d\n",dp->rec_len);
          //printf("cp: %d\n",cp);
          //getchar();
          check =1;
        }
        if(check==1)
        {
          printf("\n");
        }
        i++;
      }
      /*
      //indirect blocks initialized
      if(mip->INODE.i_block[12]!=0)
      {
        indblk = mip->INODE.i_block[12]; //get block number stored
        memset(bufrmc,0,BLKSIZE);
        getblock(mip->dev,indblk,buf);//get block into memory
        indirect = (unsigned long *) buf;//in an indirect 
        for(j=0; j<256; j++)//for each sub block in the indirect block total of 256
        {
          if(*indirect!=0)//if the block is initialized 
          {
            //print out conents

          }
        }
      }
      //double indirect blocks initialized
      if(mip->INODE.i_blocks[13]!=0)
      {

      }
*/
    
        
      //}
      
    }
    //symbolic link
    if(S_ISLNK(mip->INODE.i_mode)!=0)
    {
      i=0;
      while(i<strlen(pathname))
      {
        if(pathname[i]=='/')//theres is at least 1 parent
        {
          pexist=1;
          break;//once parents existence is confirmed exit while to save time
        }
        i++;
      }
      if(pexist==1)//parents exist,get parent and child
      {
        parent = dirname(pathname);
        child = basename(holder);
        printf("ls: %s/%s->",parent,child);
      }
      if(pexist==0)//no parents,get child
      {
        child = basename(holder);
        printf("ls: %s->",child);
      }
      memset(buf,0,BLKSIZE);
      get_block(dev,mip->INODE.i_block[0],buf);
          dp = (DIR*) buf;
          printf("%s\n",dp->name);
      
    }
    iput(mip);  
}


int chdir(char *pathname)
{
  MINODE *mip;
  DIR *dp;
  int ino=0;
  int i=0;
  int n=0;
  char *word;
  char *cp;
  char path[256];
  char paths[256][256];
 
  

    //determine initial dev: 
    if(pathname[0]== '/')
    {
      dev = root->dev;
    } 
    else
    {
      dev = running->cwd->dev;
    }                

    //convert pathname to (dev, ino);
     ino=getino(dev,pathname);
     printf("ino: %d\n",ino);
  

    //get a MINODE *mip pointing at a minode[ ] with (dev, ino);
    mip=iget(dev,ino);

    
    //if mip->INODE is NOT a DIR: reject and print error message;
    if(S_ISDIR(mip->INODE.i_mode)==0)
    {
      printf("No such directory\n");
      iput(mip);
      return;
    }
    //mip->INODE is a DIR{
    if(S_ISDIR(mip->INODE.i_mode)!=0) 
    {
      //dispose of OLD cwd;
      iput(running->cwd);
      //set cwd to mip;
      running->cwd = mip;
    }
    //printf("new directory:")
    return;
}   

int pwd(MINODE *cwd) //YOU WRITE CODE FOR THIS ONE!!!
{
  //?
  if(cwd==root)
  {
    printf("/");
  }
  else
  {
    printf("/");
    pwdinnards(cwd);
  }
  printf("\n");

  
}

pwdinnards(MINODE *cwd)
{
  /* for cwd find its parent directory, then have that directory find its parent
    directory and repeat this process until at the root*/

  MINODE *mp;//minode pointer
  DIR *dp;
  int ino=0;
  int i=0;
  int pino=0;//parent inode
  char *cp;
  char *pp;
  char name[BLKSIZE];//current directory's name
  char asd[BLKSIZE];//buffer

  //printf("before check\n");
  //at the root
  if(cwd==root)
  {
    //printf("hit root\n");
    printf("/");
    return;//stop going deeper
  }
  //printf("after check\n");

    ino = cwd->ino;
    //printf("ino: %d\t",ino);
    pino=search(cwd,"..");//find parent inode_number given the device and name of parent
    //printf("pino: %d\n",pino);
    mp= iget(cwd->dev,pino);//get the MINODE of the parent
    pwdinnards(mp);//recursivly print out the name for each parent directory which is what will happen for the rest of this function. 
    //get_block(mp->dev,pino,name);
    while(i<12)
    {//?
      //printf("i<11\n");
      get_block(dev,mp->INODE.i_block[i],name);//get the blocks of the inode in question
      dp = (DIR *)name;
      cp = name;
      //printf("&buf[BLKSIZE]: %d\n",&buf[BLKSIZE]);
      //printf("cp: %d\n",cp);
      //printf("dp->rec_len: %d\n",dp->rec_len);
      //getchar();
        //?
      if(dp->rec_len==0)//at an empty block
      {
        break;
      }
      while(cp<&name[BLKSIZE])//while cp remains less than the size of the block being examined
      {
        
        if(dp->rec_len==0)//empty block stop
        {
          break;
        }
        strncpy(asd,dp->name,dp->name_len);//copy name into bufls 
        asd[dp->name_len]=0;
        
        if(ino==dp->inode)//at correct block location
        {
          printf("%s\t",asd);//print name
          break; 
        }
        cp += dp->rec_len;//move cp further down the block in question by length of the (dir/file) it was just inspecting i.e rec_len
        dp = (DIR *) cp;//set dp to new cp location
        
      }
      i++;
      
    }
    iput(mp);
    
}

int quit()
{
  int i=0;
  /*
  for each minode[ ] do {
      if  minode[ ]'s refCount != 0: 
          write its INODE back to disk; 
  }*/

  while(i<100)
  {
    if(minode[i].refCount!=0)
    {
      iput(&minode[i]);
    }
    i++;
  }
  exit(0);  // terminate program
}

make_dir()
{
  MINODE *pip;
  MINODE *mip;
  DIR *dp;
  char *cp;
  char holder[64];
  int dev;
  int check=0;
  int pino;//parent inode number
  char *parent;
  char *child; 
  printf(".........MAKE_DIR().........\n");

  //no pathname given
  if(pathname[0]=='\0')
  {
    printf("No pathname specifed\n");
    return;
  }
  memset(holder,0,64);
  strcpy(holder,pathname);
  strcat(holder,"/");
  
  if(pathname[0]=='/')
  {
    dev = root->dev;
    //mip = root;
  }
  else
  {
    dev = running->cwd->dev;
    //mip = running->cwd;
    printf("cwd->dev: %d\n",dev);
  }
   
  parent = dirname(pathname);
  child = basename(holder);
  
  printf("parent: %s\n",parent);
  printf("child: %s\n",child);
  

  //Get the in memory of minode of parent
  //&dev
  pino = getino(dev,parent);
  pip = iget(dev,pino);

  printf("pino %d\n",pino);

  //verify parent inode is a directory
  if(S_ISDIR(pip->INODE.i_mode)==0)
  {
    printf("Parent is not a directory\n");
    return;
  }

  //verify that directory doesnt have the new child already
  check = search(&pip->INODE,child);
  if(check!=0)//found name
  {
    printf("%s already exist\n",child);
  }
  else//didnt find name
  {
    //call mymkdir(pip,child)
    mymkdir(pip,child);

    //inc parent inodes link count by 1
    pip->INODE.i_links_count++;
    //touch its atime and mark it DIRTY
    pip->INODE.i_atime = time(0L);
    pip->dirty = 1;
  }

  iput(pip);
  
}

int mymkdir(MINODE *pip, char *name)
{
  MINODE *mip;
  INODE *ip;
  DIR *dp;
  char *cp;
  int dev;
  int ino;
  int bno;
  int i;
  int need_length;
  int temp;

  printf("..............mymkdir.........\n");
  dev= pip->dev;

  ino = ialloc(dev);
  //bno = balloc(dev);
  bno = balloc(dev);
  printf("new ino: %d\n",ino);

  // 3, 4, 5,
  mip = iget(dev,ino);
  mip->INODE.i_block[0]=bno;
  ip = &mip->INODE;

  ip->i_mode = 0x41ED;
  ip->i_uid = running->uid;
  ip->i_gid = running->gid;
  ip->i_size = BLKSIZE;
  ip->i_links_count = 2;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2;
  //ip->i_block[0] = bno; cupcake
  ip->i_block[1] = 0;
  ip->i_block[2] = 0;
  ip->i_block[3] = 0;
  ip->i_block[4] = 0;
  ip->i_block[5] = 0;
  ip->i_block[6] = 0;
  ip->i_block[7] = 0;
  ip->i_block[8] = 0;
  ip->i_block[9] = 0;
  ip->i_block[10] = 0;
  ip->i_block[11] = 0;
  ip->i_block[12] = 0;
  ip->i_block[13] = 0;
  ip->i_block[14] = 0;
  mip->dirty = 1;
  //iput(mip);

  //create the . and .. directories for the new directory
  memset(buf,0,BLKSIZE);//clean out buf
  get_block(dev,ino,buf);
  dp=(DIR*)buf;//set up buf to be a reference to a directory
  dp->inode= ino;//set directories inode number to its own inode number.
  strncpy(dp->name,".",1);//copy name, name lenth, and rec lenth into the directory
  dp->name_len = 1;
  dp->rec_len=12;

  //move to the next location to add the ".." directory
  cp =buf;
  cp += dp->rec_len;
  dp = (DIR*)cp;

  //the ".." references the parent of the current directory so set its inode number to the parents inode number.
  dp->inode = pip->ino;
  strncpy(dp->name,"..",2);
  dp->name_len=2;
  dp->rec_len=BLKSIZE-12;//its rec_len should cover the remaining space minus the space for the "." directory until another directory is added.
  put_block(dev,bno,buf);// write buf[] to the disk block bno
  enter_name(pip,ino,name);
  iput(mip);
 
}

int enter_name(MINODE *pip, int myino, char *myname)
{
  DIR *dp;
  char *cp;
  int ino=0;
  unsigned long *indirect;
  unsigned long *doubleIndirect;
  int ideal_len=0;
  int need_len=0;
  int name_len=0;
  int rec_len=0;
  int remain=0;
  int n_len=0;
  int i=0;
  int blk=0;
  int bno=0;
  int temp=0;


  printf("..........enter_name........\n");
  n_len = strlen(myname);
  while(i<12)
  {
   
    if(pip->INODE.i_block[i]!=0)//block allocated
    {
      //get parent's data block into a buf
      memset(buf,0,BLKSIZE);
      get_block(pip->dev,pip->INODE.i_block[i],buf);
      dp = (DIR *)buf;
      cp = buf;

      //calculated needed length(n_len?) same as the ideal_len
      need_len = 4*((8 + n_len +3)/4);

      //Step to the last entry in block:
      blk = pip->INODE.i_block[i];
      printf("step to LAST entry in data block %d\n",blk);
      //make sure buf1 is empty
      memset(buf1,0,BLKSIZE);
      get_block(pip->dev,pip->INODE.i_block[i],buf1);
      dp = (DIR *)buf1;
      cp = buf1;

      while (cp + dp->rec_len < buf1 + BLKSIZE)
      {
        printf("%s/",dp->name);
        cp += dp->rec_len;
        dp = (DIR *)cp; 
      } 
      printf("\n");
      // dp now points to last entry in block
      printf("name: %s\trec_len: %d\n",dp->name,dp->rec_len);
      //calculate rec length
      rec_len = dp->rec_len;
      ideal_len = 4*((8+dp->name_len + 3)/4);
      printf("ideal_len: %d\n",ideal_len);
      printf("rec_len: %d\n",rec_len);
      remain = rec_len -ideal_len;
      printf("remain: %d\n",remain);
      printf("need_len: %d\n",need_len);
  
      //if there is enough space for the new directory
      if(remain>=need_len)
      {
        //enter new entry as the last entry and trim the previous entry to its ideal length
        printf("theres enough space\n");
        //trim the previous entry to its ideal_length
        printf("trimming previous entry to its ideal_length\n");
        //trim previous entry
        dp->rec_len = ideal_len;
        cp += dp->rec_len;
        dp = (DIR*)cp;
      
        //enter new entry
        dp->inode=myino;
        dp->rec_len=remain;//new line
        dp->name_len = strlen(myname);
        strncpy(dp->name, myname, dp->name_len);
        //Write data block back to disk
        put_block(pip->dev,pip->INODE.i_block[i],buf1);
        break;
      }
    }
    else//block isnt allocated
    { //no space left
      printf("no space left, allocating new block\n");
      //Reaching here means theres no space left in the data block
      //Allocate a new data block
      //bno = balloc(pip->dev);
      bno = pip->INODE.i_block[i]=balloc(pip->dev);
      //printf("bno: %d\n",bno);
      //inc parents i_size by 1024
      pip->INODE.i_size += 1024;
      memset(buf2,0,BLKSIZE);
      //enter new entry as the first entry
      get_block(pip->dev,bno,buf2);
      dp = (DIR *)buf2;
      cp = buf2;
      dp->inode = myino;//set new ino
      printf("dp->inode: %d\n",dp->inode);
      dp->rec_len = BLKSIZE;// set new rec_len
      printf("dp->rec_len: %d\n",dp->rec_len);
      dp->name_len = strlen(myname); //set new name_len
      printf("dp->name_len: %d\n",dp->name_len);
      strncpy(dp->name,myname,dp->name_len);//set new name
      printf("dp->name: %s\n",dp->name);
      put_block(pip->dev,bno,buf2);
      break;
    }
        
    i++;
  }
}

int creat_file(char *pathname)
{
  MINODE *pip;
  MINODE *mip;
  DIR *dp;
  char *cp;
  int dev;
  int check=0;
  int pino;//parent inode number
  char *parent;
  char *child; 
  char holder[64];
 
  printf(".........creat_file().........\n");
  //no pathname given
  if(pathname[0]=='\0')
  {
    printf("No pathname specifed\n");
    return;
  }
  memset(holder,0,64);
  strcpy(holder,pathname);
  strcat(holder,"/");
  if(pathname[0]=='/')
  {
    dev = root->dev;
    mip = root;
  }
  else
  {
    dev = running->cwd->dev;
    mip = running->cwd;
    printf("cwd->dev: %d\n",dev);
  }

  child = basename(holder);  
  parent = dirname(pathname);

  printf("parent: %s\n",parent);
  printf("child: %s\n",child);
  

  //Get the in memory of minode of parent
  //&dev
  pino = getino(dev,parent);
  pip = iget(dev,pino);

  printf("pino %d\n",pino);

  //verify parent inode is a directory
  if(S_ISDIR(pip->INODE.i_mode)==0)
  {
    printf("Parent is not a directory\n");
    return;
  }

  //verify that directory doesnt have the new child already
  check = search(&pip->INODE,child);
  if(check!=0)//found name
  {
    printf("%s already exist\n",child);
  }
  else//didnt find name
  {
    //call mymkdir(pip,child)
   my_creat(pip,child);

    //touch its atime and mark it DIRTY
    pip->INODE.i_atime = time(0L);
    pip->dirty = 1;
  }

  iput(pip);
  
  
}
int my_creat(MINODE *pip, char *name)
{
  MINODE *mip;
  INODE *ip;
  DIR *dp;
  char *cp;
  int dev;
  int ino;
  int bno;
  int i;
  int need_length;
  int temp;

  printf("..............my_creat.........\n");
  dev= pip->dev;

  ino = ialloc(dev);
  //bno = balloc(dev);
  bno = balloc(dev);
  printf("new ino: %d\n",ino);

  // 3, 4, 5,
  mip = iget(dev,ino);
  mip->INODE.i_block[0]=bno;
  ip = &mip->INODE;

  ip->i_mode = 0x81A4;
  ip->i_uid = running->uid;
  ip->i_gid = running->gid;
  ip->i_size = BLKSIZE;
  ip->i_links_count = 1;
  ip->i_atime = time(0L);
  ip->i_ctime = time(0L);
  ip->i_mtime = time(0L);
  ip->i_blocks = 0;
  ip->i_block[0] = 0;
  ip->i_block[1] = 0;
  ip->i_block[2] = 0;
  ip->i_block[3] = 0;
  ip->i_block[4] = 0;
  ip->i_block[5] = 0;
  ip->i_block[6] = 0;
  ip->i_block[7] = 0;
  ip->i_block[8] = 0;
  ip->i_block[9] = 0;
  ip->i_block[10] = 0;
  ip->i_block[11] = 0;
  ip->i_block[12] = 0;
  ip->i_block[13] = 0;
  ip->i_block[14] = 0;
  mip->dirty = 1;
  iput(mip);

  //create the . and .. directories for the new directory
  memset(buf,0,BLKSIZE);//clean out buf
  get_block(dev,ino,buf);
  dp=(DIR*)buf;//set up buf to be a reference to a directory
  dp->inode= ino;//set directories inode number to its own inode number.
  strncpy(dp->name,".",1);//copy name, name lenth, and rec lenth into the directory
  dp->name_len = 1;
  dp->rec_len=12;

  //move to the next location to add the ".." directory
  cp =buf;
  cp += dp->rec_len;
  dp = (DIR*)cp;

  //the ".." references the parent of the current directory so set its inode number to the parents inode number.
  dp->inode = pip->ino;
  strncpy(dp->name,"..",2);
  dp->name_len=2;
  dp->rec_len=BLKSIZE-12;//its rec_len should cover the remaining space minus the space for the "." directory until another directory is added.
  put_block(dev,bno,buf);// write buf[] to the disk block bno
  enter_name(pip,ino,name);
  iput(mip);
}

int rmdir()
{
  MINODE *pip;
  MINODE *mip;
  INODE *ip;
  DIR *dp;
  char *cp;
  char *sp;
  char *parent;
  char *child;
  char holder[64];
  unsigned long *indirect;
  unsigned long *doubleIndirect;
  int j=0;
  int uid=0;
  int dev;
  int ino;
  int pino;
  int bno;
  int i;
  int need_length;
  int temp;
  

  printf("..............rmdir()..........\n");
  //if no pathname given
  if(pathname[0]=='\0')
  {
    printf("No pathname specified\n");
    return;
  }
  memset(holder,0,64);
  strcpy(holder,pathname);
  strcat(holder,"/");
  //determine dev
  if(pathname[0]=='/')
  {
    dev = root->dev;
  }
  else
  {
    dev = running->cwd->dev;
  }
  
  child = basename(holder);  
  parent = dirname(pathname);

  //2. get inumber of pathname: 
  ino = getino(dev, pathname);
 
  //3. get its minode[ ] pointer:
  mip = iget(dev, ino);
  printf("mip ino: %d\n",mip->ino);
  printf("mip->refCount: %d\n",mip->refCount);

  //4. check ownership(?)
  /*
  memset(buf,0,BLKSIZE);
  get_block(dev,ino, buf);
  if()
  */
  //////////////////////////////////////////////////////////////////////////////////HERE
  //5. check DIR type and not busy and is empty:
  if( mip->INODE.i_mode!=0x41ED)//not a directory
  {
    iput(mip);
    printf("The file is not a directory\n");
    return;
  }
  if( mip->refCount>1)//busy
  {
    iput(mip);
    printf("The directory is busy\n");
    return;
  }
  //empty? not empty if ".." doesnt have a rec_len of 1012(the size of a block minus "." "..")
  memset(buf,0,BLKSIZE);
  get_block(dev,mip->INODE.i_block[0],buf);
  dp = (DIR*) buf;
  cp = buf;
  cp += dp->rec_len;
  dp = (DIR*) cp;
  if(dp->rec_len!=1012)
  {
    iput(mip);
    printf("The directory isn't empty \n");
    return;
  }
  else//dp is at the .. entry of the targeted directory so use it to get the parents ino.
  {
    pino=dp->inode;
    printf("pino: %d\n",pino);
  }
  //6. Deallocate its block and inode 
  for (i=0; i<12; i++)
  {
    if(mip->INODE.i_block[i]==0)
    {
      continue;
    }
    bdealloc(mip->dev,mip->INODE.i_block[i]);//deallocate block
  }
  idealloc(mip->dev,mip->ino);//deallocate inode
  //indirect
  if(mip->INODE.i_block[12]!=0)
  {

  }
  iput(mip);//clears mip->refCount to 0, puts it back

  //7. get parent DIR's ino and Minode pointed by pip
  pip = iget(mip->dev,pino);

  //8. remove child's entry from parent directory
  rm_child(pip,child);

  //9. Finish up
  pip->INODE.i_links_count--;
  pip->INODE.i_atime=time(0L);
  pip->INODE.i_mtime=time(0L);
  pip->dirty=1;
  iput(pip);
  return;
}
int rm_child(MINODE *pip, char *name)//removes child's name reference from parent directory
{
  DIR *dp;
  char *prev;
  char *cp;
  char *end;
  char *start;
  int used =0;
  int i=0;
  int g=0;
  int temp=0;
  int check = 1;
  int found=0;

  //NOTE: like the search function this function is in charge of going through the i_blocks of the parent inode
  //      and removing the entry of the specified name. 
  printf(".........rm_child().........\n");
  while(i<12)//direct blocks only
  {
    //found=0;
    used =0;
    //get the block to be examined
    //printf("getting block %d\n",i);
    memset(buf1,0,BLKSIZE);//clear out buf1
    get_block(pip->dev,pip->INODE.i_block[i],buf1);
    dp = (DIR*)buf1;
    prev = buf1;
    cp = buf1;
    start = buf1;
    end = buf1;
    //getchar();
    //make sure the block is activated
    if(pip->INODE.i_block[i]!=0)
    {
      //find the last entry in the block
      while(end+dp->rec_len<&buf1[BLKSIZE])
      {
        //printf("name: %s\n",dp->name);
        end += dp->rec_len;
        dp = (DIR*)end;
      }//after this end should point to the last possible location in the block
      //printf("last item in block[%d]: %s\n",i,dp->name);
      
      printf("checking if empty\n");
      //getchar();
      if(cp==end)//there is only one entry in the block
      {
        printf("only item\n");
        g=i;
        bdealloc(pip->dev,pip->INODE.i_block[i]);//deallocate block 
        while(g<12)
        {
          pip->INODE.i_block[g]=pip->INODE.i_block[g+1];//shift blocks over
          g++;
        }
      }
      else
      {
        printf("not empty\n");
        //getchar();
        dp = (DIR*)cp; //set dp back to the begining of the block
        while(cp<&buf1[BLKSIZE])//while current pointer is still within the block being examined
        {
          strncpy(bufrmc,dp->name,dp->name_len);
          bufrmc[dp->name_len] = 0;
          printf("name being examined: %s\n",bufrmc);
          check = strcmp(bufrmc,name);
          temp =dp->rec_len;
          if(check==0)//FOUND name
          {
            //found=1;
            //printf("found %s\n",name);
            //getchar();
            if(cp==end)//name to be removed is at the end of the block
            {
             // printf("found at the end\n");
              dp =(DIR *)prev;
              dp->rec_len += temp;
              //printf("new last name:%s temp: %d\n",dp->name,temp);
              //put_block(pip->dev,pip->INODE.i_block[i],buf1);
              //break;
            } 
            else//name to be removed is in the middle of the block somewhere
            {
              //printf("found in the middle\n");
              //old
              dp = (DIR*)end;
              dp->rec_len += temp;
              memcpy(cp,cp+temp,BLKSIZE-used-temp);//copies everything else in the block to the left
              //put_block(pip->dev,pip->INODE.i_block[i],buf1);
              //break;
              
            }
            break;
          }
          prev = cp;
          cp += dp->rec_len;
          used += dp->rec_len;
          dp = (DIR *)cp;         
        }
        /*
        if(dp->rec_len==0)//inside of empty block, havent found name  
        {
          printf("Shouldn't be here (inside rm_child\n");
          break;//exit search
        }  
        */
      }
    }
    //Write the parent's data block back to disk(not activated)
  
    put_block(pip->dev,pip->INODE.i_block[i],buf1);   
    i++;
  }
  //mark parent minode dirty
  pip->dirty = 1;

}
int rm()
{
  MINODE *pip;
  MINODE *mip;
  char *parent;
  char *child;
  char holder[64];
  int pino;
  int cino;
  int dev;
  int i;
  //check user input
  if(pathname=="")
  {
    printf("ERROR: no pathname specifed\n");
    return;
  }
  memset(holder,0,64);
  strcpy(holder,pathname);
  strcat(holder,"/");

  //get dev
  if(pathname[0]=='/')
  {
    dev = root->dev;
  }
  else
  {
    dev = running->cwd->dev;
  }

  parent=dirname(pathname);
  child=basename(holder);

  printf("parent: %s\n",parent);
  printf("child: %s\n",child);

  //getino
  pino = getino(dev,parent);
  cino = getino(dev,holder);

  //does it exist
  printf("pino: %d\n",pino);
  printf("cino: %d\n",cino);
  if((pino==0)||(cino==0))
  {
    printf("ERROR: pathname doesnt exist\n");
    return;
  }
  //get INODE into memory
  mip = iget(dev,cino);
  //is it a file
  if(S_ISREG(mip->INODE.i_mode)==0)//
  {
    printf("ERROR: file is not a REG file\n");
    iput(mip);
    return;
  }
  pip = iget(dev,pino);
  //is it a dir 
  if(S_ISDIR(pip->INODE.i_mode)==0)
  {
    printf("ERROR: pathname doesnt exist\n");
    iput(pip);
    iput(mip);
  }
  //remove file
  for (i=0; i<12; i++)
  {
    if(mip->INODE.i_block[i]==0)
    {
      continue;
    }
    bdealloc(mip->dev,mip->INODE.i_block[i]);//deallocate block
  }
  idealloc(mip->dev,mip->ino);//deallocate inode
  iput(mip);//clears mip->refCount to 0, puts it back
  rm_child(pip,child);
  //write back
  iput(pip);
}


int link(char* oldpath,char *newpath)
{
  MINODE *nip;
  MINODE *oip;
  int oldDev;
  int newDev;
  int oldIno;
  int newParentIno;
  char *oldParent;
  char *oldChild;
  char *newParent;
  char *newChild;
  char holder[64];
  int check=0;
  

  printf("......... link() .........\n");
  //check for user issues
  if((oldpath=="")||(newpath==""))
  {
    printf("ERROR: a pathname was unspecified\n");
  }
  memset(holder,0,64);
  strcpy(holder,newpath);
  strcat(holder,"/");
  //get old dev
  if(oldpath[0]=='/')
  {
    oldDev = root->dev;
  }
  else
  {
    oldDev = running->cwd->dev;
  }
  //get new dev
  if(newpath[0]=='/')
  {
    newDev = root->dev;
  }
  else
  {
    newDev = running->cwd->dev;
  }
  //make sure there the same 
  if(oldDev!=newDev)
  {
    printf("ERROR: paths must be on same device\n");
    return;
  }
  
  //make sure oldpath is legitimate
  oldIno = getino(oldDev,oldpath);
  if(oldIno ==0)
  {
    printf("ERROR: oldpath doesnt exist\n");
    return;
  }
  oip = iget(oldDev,oldIno);
  if(S_ISDIR(oip->INODE.i_mode)!=0)
  {
    printf("ERROR: cannot link to a directory!\n");
    iput(oip);
    return;
  }
  //make sure newpath parent dir is legitimate
  newParent = dirname(newpath);
  newParentIno = getino(newDev,newParent);
  newChild = basename(holder);
  //determine if name already exist in newParent
  nip=iget(newDev,newParentIno);
  if(S_ISDIR(nip->INODE.i_mode)==0)//parent path isnt a directory
  {
    printf("ERROR: newpath doest exist\n");
    iput(oip);
    iput(nip);
    return;
  }
  check = search(&nip->INODE,newChild);
  if(check!=0)//found name
  {
    printf("ERROR: file name already exist\n");
    iput(oip);
    iput(nip);
    return;
  }
  //creat file at end of newpath (newChild) and set its inode to oldIno;
  my_link(nip,newChild,oldIno);
  
  //increment linkcount by one
  oip->INODE.i_links_count++;
  oip->dirty=1;
  iput(oip);
  iput(nip);

}
int my_link(MINODE *pip, char *name,int pino)
{
  MINODE *mip;
  INODE *ip;
  DIR *dp;
  char *cp;
  int dev;
  int ino;
  int bno;
  int i;
  int need_length;
  int temp;

  printf("............my_link.........\n");
  dev= pip->dev;

  ino = pino;
  //bno = balloc(dev);
  bno = balloc(dev);
  //printf("new ino: %d\n",ino);

  // 3, 4, 5,
  mip = iget(dev,ino);
  mip->INODE.i_block[0]=bno;
  ip = &mip->INODE;

  ip->i_mode = 0x81A4;
  ip->i_uid = running->uid;
  ip->i_gid = running->gid;
  ip->i_size = BLKSIZE;
  ip->i_links_count = 1;
  ip->i_atime = time(0L);
  ip->i_ctime = time(0L);
  ip->i_mtime = time(0L);
  ip->i_blocks = 0;
  ip->i_block[0] = 0;
  ip->i_block[1] = 0;
  ip->i_block[2] = 0;
  ip->i_block[3] = 0;
  ip->i_block[4] = 0;
  ip->i_block[5] = 0;
  ip->i_block[6] = 0;
  ip->i_block[7] = 0;
  ip->i_block[8] = 0;
  ip->i_block[9] = 0;
  ip->i_block[10] = 0;
  ip->i_block[11] = 0;
  ip->i_block[12] = 0;
  ip->i_block[13] = 0;
  ip->i_block[14] = 0;
  mip->dirty = 1;
  iput(mip);

  //create the . and .. directories for the new directory
  memset(buf,0,BLKSIZE);//clean out buf
  get_block(dev,ino,buf);
  dp=(DIR*)buf;//set up buf to be a reference to a directory
  dp->inode= ino;//set directories inode number to its own inode number.
  strncpy(dp->name,".",1);//copy name, name lenth, and rec lenth into the directory
  dp->name_len = 1;
  dp->rec_len=12;

  //move to the next location to add the ".." directory
  cp =buf;
  cp += dp->rec_len;
  dp = (DIR*)cp;

  //the ".." references the parent of the current directory so set its inode number to the parents inode number.
  dp->inode = pip->ino;
  strncpy(dp->name,"..",2);
  dp->name_len=2;
  dp->rec_len=BLKSIZE-12;//its rec_len should cover the remaining space minus the space for the "." directory until another directory is added.
  put_block(dev,bno,buf);// write buf[] to the disk block bno
  enter_name(pip,ino,name);
  iput(mip);
}

int unlink(char *path)//link unlink
{ 
  MINODE *mip;
  MINODE *pip;
  int dev;
  int pino;
  int cino;
  char *parent;
  char *child;
  char holder[64];
 

  printf(".......unlink().......\n");
  //check user input
  if(path=="")
  {
    printf("ERROR: no pathname specified\n");
    return;
  }
   memset(holder,0,64);
  strcpy(holder,path);
  strcat(holder,"/");
  //get dev
  if(path[0]=='/')
  {
    dev = root->dev;
  }
  else
  {
    dev = running->cwd->dev;
  }
  //make sure path is legitimate
  child = basename(holder);
  parent = dirname(path);
  printf("parent: %s\n",parent);
  printf("child: %s\n",child);

  pino = getino(dev,parent);
  cino = getino(dev,path);
  printf("pino: %d\n",pino);
  printf("cino: %d\n",cino);

  if((cino==0)||(pino==0))
  {
    printf("ERROR: pathname doesnt exist \n");
    return;
  }
  //1. get pathnames Inode into memory
  pip = iget(dev,pino);
  mip = iget(dev,cino);
  
   //2. verify its a File (REG) (LNK) can not be a directory
  if(S_ISDIR(mip->INODE.i_mode)!=0)
  {
    printf("ERROR: file is a directory\n");
    iput(mip);
    return;
  }
 
  //3. decrement INODEs i_links_count by 1
  mip->INODE.i_links_count--;
  //4. if i_links_count== 0 ==> rm pathname by deallocate it data blocks truncate(INODE)
  if(mip->INODE.i_links_count==0)
  {
    Truncate(mip);
    iput(mip);
  }
  //5. Remove childName = basename(pathname) from the parent directory by
  //  rm_child(parentInodePtr, childName)
  rm_child(pip,child);
  iput(pip);
}
int Truncate(MINODE *mip)
{
  printf(".......Truncate().......\n");
  int i=0;
  //if the i_links_count was already recognized as 0 we dont need to check to make sure it empty
  for (i=0; i<12; i++)
  {
    if(mip->INODE.i_block[i]==0)
    {
      continue;
    }
    bdealloc(mip->dev,mip->INODE.i_block[i]);//deallocate block
  }
  idealloc(mip->dev,mip->ino);//deallocate inode

  return;

}

int symlink(char *oldpath, char *newpath)
{
  MINODE *oip;
  MINODE *nip;
  MINODE *kip;
  char *newParent;
  char *newChild;
  char holder[64];
  int oldDev;
  int oldIno;
  int newPino;
  int newCino;
  int newDev;
  int check=0;
  
  printf(".......symlink().......\n");
  //check user input
  if((oldpath=="")||(newpath==""))
  {
    printf("ERROR: a pathname was unspecified\n");
    return;
  }
  memset(holder,0,64);
  strcpy(holder,newpath);
  strcat(holder,"/");
  //get devs
  if(oldpath[0]=='/')
  {
    oldDev=root->dev;
  }
  else
  {
    oldDev=running->cwd->dev;
  }
  if(newpath[0]=='/')
  {
    newDev=root->dev;
  }
  else
  {
    newDev=running->cwd->dev;
  }
  //verify oldpath exist and is either DIR or REG
  oldIno = getino(oldDev,oldpath);
  printf("oldIno: %d\n",oldIno);
  if(oldIno==0)
  {
    printf("ERROR: oldpath doesnt exist\n");
    return;
  }
  oip = iget(oldDev,oldIno);
  if(S_ISREG(oip->INODE.i_mode)==0)//make sure its a REG or a DIR
  {
    if(S_ISDIR(oip->INODE.i_mode)==0)
    {
      printf("ERROR: oldpath is not a REG or DIR\n");
      iput(oip);
      return;
    }
  }
  //verify newpath exist and file name doesnt exist already
  newParent = dirname(newpath);
  newChild = basename(holder);
  newPino = getino(newDev,newParent);
  printf("newParent: %s\n",newParent);
  printf("newChild: %s\n",newChild);
  printf("newPino: %d\n",newPino);
  if(newPino==0)
  {
    printf("ERROR: newpath doesnt exist\n");
    iput(oip);
    return;
  }
  getchar();
  nip = iget(newDev,newPino);
  printf("ino: %d\n",nip->ino);
  getchar();
  if(S_ISDIR(nip->INODE.i_mode)==0)//parent path exist but is not a dir
  {
    printf("ERROR: newpath doesnt exist\n");
    iput(oip);
    iput(nip);
    return;
  }
  //verify that directory doesnt have the new child already
  check = search(&nip->INODE,newChild);
  if(check!=0)//found name
  {
    printf("ERROR: file name already exist\n");
    iput(oip);
    iput(nip);
    return;
  }
  //create a file for newpath and set its type to 0xA000
  getchar();
  my_symlink(nip,newChild,oldpath);
  newCino = getino(newDev,newChild);
  if(newCino==0)
  {
    printf("ERROR: newChild not installed correctly\n");
    iput(nip);
    iput(oip);
    return;
  }
  getchar();
  printf("kip\n");
  kip = iget(newDev,newCino);
  //write string oldpath int i_blocks //assume 60 char
  getchar();
  printf("enter name2\n");
  enter_name(kip,newCino,oldpath);
  //write INODE of newpath back to disc
  iput(nip);
  iput(oip); 
}
int my_symlink(MINODE *pip, char *name,char *name2)
{
  MINODE *mip;
  INODE *ip;
  DIR *dp;
  char *cp;
  int dev;
  int ino;
  int bno;
  int i;
  int need_length;
  int temp;

  printf("............my_symlink.........\n");
  dev= pip->dev;

  ino = ialloc(dev);
  //bno = balloc(dev);
  bno = balloc(dev);
  //printf("new ino: %d\n",ino);

  // 3, 4, 5,
  mip = iget(dev,ino);
  mip->INODE.i_block[0]=bno;
  ip = &mip->INODE;

  ip->i_mode = 0xA000;
  ip->i_uid = running->uid;
  ip->i_gid = running->gid;
  ip->i_size = BLKSIZE;
  ip->i_links_count = 1;
  ip->i_atime = time(0L);
  ip->i_ctime = time(0L);
  ip->i_mtime = time(0L);
  ip->i_blocks = 1;
  ip->i_block[1] = 0;
  ip->i_block[2] = 0;
  ip->i_block[3] = 0;
  ip->i_block[4] = 0;
  ip->i_block[5] = 0;
  ip->i_block[6] = 0;
  ip->i_block[7] = 0;
  ip->i_block[8] = 0;
  ip->i_block[9] = 0;
  ip->i_block[10] = 0;
  ip->i_block[11] = 0;
  ip->i_block[12] = 0;
  ip->i_block[13] = 0;
  ip->i_block[14] = 0;
  mip->dirty = 1;
  iput(mip);

  get_block(dev,bno,buf);
  dp=(DIR*)buf;//set up buf to be a reference to a directory
  dp->inode= ino;//set directories inode number to its own inode number.
  strncpy(dp->name,name2,strlen(name2));//copy name, name lenth, and rec lenth into the directory
  dp->name_len = strlen(name2);
  dp->rec_len=BLKSIZE;
  put_block(dev,bno,buf);
  printf("enter name 1\n");
  getchar();
  enter_name(pip,ino,name);
  iput(mip);
}

int readlink()
{
  MINODE *mip;
  DIR *dp;
  char *cp;
  int dev;
  int ino;

  //check user input
  if(pathname=="")
  {
    printf("ERROR: no pathname specified\n");
    return;
  }
  //get dev
  if(pathname[0]=='/')
  {
    dev = root->dev;
  }
  else
  {
    dev = running->cwd->dev;
  }
  //make sure path exist
  ino = getino(dev,pathname);
  if(ino==0)
  {
    printf("ERROR: pathname doesnt exist\n");
    return;
  }
  //get Inode of pathname into a minode
  mip = iget(dev,ino);
  //check Inode is a symbolic link file
  if(S_ISLNK(mip->INODE.i_mode)==0)
  {
    printf("ERROR: file is not a symbolic link\n");
    iput(mip);
    return;
  }
  //return its string contents 
  memset(buf,0,BLKSIZE);
  get_block(dev,mip->INODE.i_block[0],buf);
  dp = (DIR*)buf;

  printf("contents: %s\n",dp->name);

  iput(mip);
}

int mychmod()//changes files mode to a new mode
{
  MINODE *mip;
  INODE *ip;
  int dev;
  char *path1;
  char *path2;
  char holder[64];
  int ino=0;
  long int newMode;


  //if user didn't input pathname
  if(pathname[0]=='\0')
  {
    printf("No pathname specifed\n");
    return;
  }
  memset(holder,0,64);
  strcpy(holder,pathname);
  strcat(holder,"/");
  //get dev
  if(pathname[0]=='/')
  {
    dev = root->dev;
  }
  else
  {
    dev = running->cwd->dev;
    printf("cwd->dev: %d\n",dev);
  }
  
  path1 = dirname(pathname);
  path2 = basename(holder);

  //get ino, minode, and inode set up
  ino = getino(dev,path2);
  printf("ino: %d\n",ino);
  mip = iget(dev, ino);
  ip = &mip->INODE;

  //show old mode
  //printf("old mode: %d\n",ip->i_mode);
  if(S_ISREG(ip->i_mode)!=0)//reg 
  {
    printf("old mode: REG\n");
  }
  if(S_ISDIR(ip->i_mode)!=0)//dir
  {
    printf("old mode: DIR\n");
  }
  if(S_ISLNK(ip->i_mode)!=0)//lnk
  {
    printf("old mode: LNK\n");
  }
  //set newMode
  newMode = strtol(path2,NULL,8);

  //change mode 
  ip->i_mode = (ip->i_mode & 0xF000) | newMode;

  //printf("new mode: %d\n",ip->i_mode);
  if(S_ISREG(ip->i_mode)!=0)//reg 
  {
    printf("new mode: REG\n");
  }
  if(S_ISDIR(ip->i_mode)!=0)//dir
  {
    printf("new mode: DIR\n");
  }
  if(S_ISLNK(ip->i_mode)!=0)//lnk
  {
    printf("new mode: LNK\n");
  }

  mip->dirty = 1;
  //write back
  iput(mip);

}
int mystat()
{
  MINODE *mip;
  INODE *ip;
  int dev;
  int ino;
  int m;//mode
  char *mo;//mode
  char *t;
  char *l;

  if(pathname[0]=='\0')
  {
    printf("No pathname specifed\n");
    return;
  }
  //get dev
  if(pathname[0]=='/')
  {
    dev = root->dev;
  }
  else
  {
    dev = running->cwd->dev;
    printf("cwd->dev: %d\n",dev);
  }

  //get minode of file
  ino = getino(dev,pathname);
  mip = iget(dev,ino);
  //get inode of file
  ip = &mip->INODE;
  //mode?
  mo = (char*)malloc(20 * sizeof(char));
  if(S_ISREG(ip->i_mode)!=0)// file
  {
    printf("mode: REG\n");
  }
  if(S_ISDIR(ip->i_mode)!=0)//directory
  {
    printf("mode: DIR\n");
  }
  if(S_ISLNK(ip->i_mode)!=0)//link
  {
    printf("mode: LNK\n");
  }
  //print information
  //printf("mode: %u\n",(unsigned short)ip->i_mode);
  //printf("mode: %s\n",mo);
  printf("Device: %d\t Inode: %d\t Links: %d\n",dev,mip->ino,ip->i_links_count);
  printf("gid: %u\t uid: %u\n",ip->i_gid,ip->i_uid);
  printf("size: %d\t nblocks: %d\t",ip->i_size,ip->i_blocks);
  t = ctime(&ip->i_ctime);
  printf("ctime:%s\n",t);
  t = ctime(&ip->i_mtime);
  printf("mtime:%s\n",t);
  t = ctime(&ip->i_atime);
  printf("atime:%s\n",t);
  
  iput(mip);
  /*
	char *time_string = ctime(&stPtr->st_ctime);
	printf("\nctime: %s", time_string);
	time_string = ctime(&stPtr->st_atime);
	printf("atime: %s", time_string);
	time_string = ctime(&stPtr->st_mtime);
  printf("mtime: %s", time_string);
  */ 
}
int mytouch()//touch function
{
  MINODE *mip;
  INODE *ip;
  char *t;
  int dev;
  int ino;
  int check=0;

  if(pathname[0]=='\0')
  {
    printf("No pathname specifed\n");
    return;
  }
  //get dev
  if(pathname[0]=='/')
  {
    dev = root->dev;
  }
  else
  {
    dev = running->cwd->dev;
    printf("cwd->dev: %d\n",dev);
  }
  //getino
  ino = getino(dev,pathname);
  //check file already exist
  if(ino!=0)//file already exist
  {
    mip = iget(dev,ino);
    ip = &mip->INODE;
    if(S_ISREG(ip->i_mode)==0)//not a file
    {
      printf("ERROR: name already exist and is not a file\n");
      iput(mip);
      return;
    }
    //is a file
    ip->i_atime = time(0L);
    ip->i_mtime = time(0L);
    t = ctime(&ip->i_atime);
    printf("new atime: %s\n",t);
    t = ctime(&ip->i_mtime);
    printf("new mtime: %s\n",t);
  }
  else//file doesnt exist
  {
    creat_file(pathname);
  }
}

///Level 2

int open_file(char *path,char *mo)
{
  MINODE *mip;
  OFT *temp;
  OFT *oftp;
  int mode=0;
  int dev;
  int ino;
  int check =0;
 //check user input
 if(path==""||mo=="");
 {
   printf("ERROR: no filename or mode given\n");
   return;
 }
 if((mo!="0") && (mo!="1") && (mo!="2") && (mo!="3"))//check for valid mode
 {
   printf("ERROR: invalid mode value\n");
   return;
 }
 //getdev
 if(pathname[0]=='/')
 {
   dev = root->dev;
 }
 else
 {
   dev = running->cwd;
 }
 //get mode
 if(mo=="0")
 {
   mode = 0;
   check =1;
 }
 if(mo=="1")
 {
   mode =1;
 }
 if(mo=="2")
 {
   mode =2;
 }
 if(mo=="3")
 {
   mode =3;
 }

 //getino
 ino = getino(dev,pathname);
 //does it exist?
 if(ino==0)
 {
   printf("ERROR: Pathname doesnt exist\n");
   return 0;
 }
 printf("ino: %d\tmode: %d\n",ino,mode);
 //get INODE into memory
 mip = iget(dev,ino);
 //check if REG file
 if(S_ISREG(mip->INODE.i_mode)==0)//not a REG file
 {
   printf("ERROR: file must be a REG file\n");
   iput(mip);
   return;
 }
 //check if file already open
 int i=0;
 while(running->fd[i])//there is a valid oft
 {
   if(running->fd[i]->mptr==mip)//right INODE
   {
     if((running->fd[i]->mode==1)||(running->fd[i]->mode==2)||(running->fd[i]->mode==3))
     {
       printf("ERROR: File is already open in Inocompatible mode\n");
       iput(mip);
       return;
     }
   } 
  i++; 
 }
 //Allocate open file table
 oftp = malloc(sizeof(OFT));
 oftp->mode = mode;
 oftp->refCount = 1;
 oftp->mptr = mip;

 //Set offset accordingly
 switch(mode)
 {
    case 0:
        oftp->offset = 0;
        check =1;
        break;
    case 1:
        mytruncate(mip);
        oftp->offset = 0;
        break;
    case 2:
        oftp->offset = 0;
        break;
    case 3:
        oftp->offset = mip->INODE.i_size; 
        break;
    default: 
        printf("ERROR: Invalid mode\n");
        return -1;
 }
 //find smallest fd[i] so that fd[i]==NULL
 i =0;
 while(running->fd[i])
 {
   i++;
 }
 printf("smallest fd: %d\n",i);

 //update Inodes time field
 if(check==1)// read mode
 {
   mip->INODE.i_atime=time(0L);
 }
 else//any other mode
 {
    mip->INODE.i_atime=time(0L);
     mip->INODE.i_mtime=time(0L);
 }
 fd = i;
 return i;
}

int mytruncate(MINODE *mip)//without checking if emtpy
{
  int i;
  int j;
  unsigned long indblk;
  unsigned long dbindblk;

  //direct blocks
  i=0;
  while(i<12)
  {
    bdealloc(mip->dev,mip->INODE.i_block[i]);//free block
   
    i++;
  }
  //indirect blocks
  indblk = mip->INODE.i_block[12];
  for(i=0; i<256; i++)
  {

  }
  mip->INODE.i_size = 0;//change size
  mip->INODE.i_atime = time(0L);
  mip->INODE.i_ctime = time(0L);
  mip->INODE.i_mtime = time(0L);
  //bdealloc(mip->dev,mip-)

  //double indirect blocks


}

int close()
{
  //1. Verify fd is within range
 //2. Verify running->fd[fd] is pointing at an OFT entry
 //3. The following code segements should be fairly obvious
 //oftp = running->fd[fd];
 //running->fd[fd]=0;
 //oftp->refCount--;
 //if(oftp->refCount>0) return 0;
 //mip = oftp->inodeptr;
 //iput(mip);
 //return 0;

}

int read_file()
{
  /* Preperations:
    ASSUME: file is opened for RD or RW;
    ask for a fd and  */
}
int pfd()
{
  OFT *temp;
  int i;
  char mode[64];
  printf(".....pfd().....\n");
  memset(mode,0,64);
  i=0;
  temp = running->fd[0];
  printf("\t fd \tmode\toffset\t INODE\n");
  printf("\t----\t----\t------\t--------");
  i=0;
  while(temp!=NULL)
  {
    i++;
    printf("\t %d \t%s\t%d\t [%d,%d]\n",i,temp->mode,temp->offset,temp->mptr->dev,temp->mptr->ino);
    temp = running->fd[i];
  }
}
int myread(int fd, char buf[ ],int nbytes)
{

}
int write()
{

}

int lseek()
{
  /*From fd, find the OFT entry. 
  change OFT entry's offset to position but make sure not to over run either end of the file
  return original position */
}

int cat()
{

}

int cp()
{

}

int mv()
{

}
