#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include <stddef.h>

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path,char* showhidden)
{
  char buf[512], *p;
  char hidden='.';
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(showhidden==NULL && de.name[0]==hidden)
            continue;
        else if(de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf(1, "ls: cannot stat %s\n", buf);
            continue;
      }
      if(st.type==2)    //refer stat.h file, #define T_FILE 2
        printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
      else if(st.type==1)  //refer stat.h file, #define T_DIR 1
        printf(1, "%s/ %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;
  char* showhidden=NULL;

  if(argc < 2){
    ls(".",showhidden);
    exit();
  }
  if(argc>=2 && strcmp(argv[1],"-a")==0){
    showhidden="showhidden";
    if(argc==2){
        ls(".",showhidden);
        exit();
    }
    else{
    for(i=2; i<argc; i++)
        ls(argv[i],showhidden);
    exit();
    }
  }
  else{
  for(i=1; i<argc; i++)
    ls(argv[i],showhidden);
  exit();
}}