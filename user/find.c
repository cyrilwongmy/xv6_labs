#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  //static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  return p;

}
void
find(char *path, char *fileName){

  int fd;
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  struct stat st;
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }


  char buf[512], *p;
  struct dirent de;
  switch(st.type){
  case T_FILE:
    if(strcmp(fmtname(path),fileName) == 0){
      printf("%s\n", path);
    }
    break;
  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path); // buf = ".";
    p = buf+strlen(buf); 
    *p++ = '/'; // buf = "./"
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ); // buf = "./README0";
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      // check whehter .. or .
      if ((strcmp(fmtname(buf), ".") != 0) && (strcmp(fmtname(buf), "..") != 0)){
        find(buf, fileName);
      }
    }
    break;
  }
  close(fd);
}


int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "find usage: find directory filename...\n");
  }
  // readin args 
  // 1. directory tree name
  // 2. target filename
  char *path = argv[1];
  char *fileName = argv[2];

  find(path, fileName);

  exit(0);
}

