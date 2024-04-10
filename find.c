#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"


void findname(char *path,char* name, int printi,char* type,char* ino){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    if((fd = open(path, 0)) < 0){
    printf(2, "find: cannot open %s\n", path);                       //checks if we can open the file
    return;
    }
    if(fstat(fd, &st) < 0){
    printf(2, "find: cannot stat %s\n", path);                       // checks if we can load the contents of the file
    close(fd);
    return;
    }
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf(1, "find: path too long\n");          
        return;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    int count=0;
    while(read(fd, &de, sizeof(de)) == sizeof(de)){                 //iteratively visits all  the files and directories
        count+=1;
        if(de.inum==0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf(1, "find: cannot stat %s\n", buf);
            continue;
        }
        if(strcmp(type,"f")==0){                                    //function for -type flag for file
            if(st.type==2){
                if(strcmp(de.name,".")!=0 && strcmp(de.name,"..")!=0){
                    if(strcmp(de.name,name)==0){
                        printf(1,"%s\n",buf);
                    }
                }
            }
            else if(st.type==1){
                if(strcmp(de.name,".")!=0 && strcmp(de.name,"..")!=0){
                findname(buf,name,printi,type,"");
                }
            }
        }
        else if(strcmp(type,"d")==0){      //function for -type flag for directory
            if(st.type==1){
                if(strcmp(de.name,".")!=0 && strcmp(de.name,"..")!=0){
                    if(strcmp(de.name,name)==0){
                     printf(1,"%s\n",buf);   
                    }
                findname(buf,name,printi,type,"");
                }
            }
        }
        else if(ino[0]!='\0'){
            if(strcmp(de.name,".")!=0 && strcmp(de.name,"..")!=0){
                if(ino[0]=='+'){                                        //prints the path of file if it exists above that inode number
                    int number=atoi(ino+1);
                    if(st.type==2){
                        if(st.ino>number && strcmp(name,de.name)==0){
                            printf(1,"%s\n",buf);
                        }
                    }
                    else if(st.type==1){
                        findname(buf,name,printi,type,ino);
                    }
                }
                else if(ino[0]=='-'){
                    int number=atoi(ino+1);
                    if(st.ino<number && strcmp(name,de.name)==0){       //prints the path of file if it exists below that inode number
                        printf(1,"%s\n",buf);                            
                    }
                    else if(st.type==1){
                        findname(buf,name,printi,type,ino);
                    }
                }
                else{   
                    int number=atoi(ino);
                    if((st.ino==number) && strcmp(name,de.name)==0){    //prints the path of file if the inode number matches
                        printf(1,"%s\n",buf);                            
                    }
                    else if(st.type==1){
                        findname(buf,name,printi,type,ino);
                    }
                }
            }
        }
        else if(strcmp(type,"")==0){                                    //if type is not specified
            if(strcmp(de.name,".")!=0 && strcmp(de.name,"..")!=0){
                    if(strcmp(de.name,name)==0){
                        if (printi==1){
                            printf(1,"%d %s\n",st.ino,buf);
                        }
                        else{
                        printf(1,"%s\n",buf);
                        }
                    }
                    if(st.type==1){
                        findname(buf,name,printi,type,"");
                    }
            }
        }
    }
    close(fd);
    exit();
}
int main(int argc, char *argv[]) {  
    if(argc==1){
        printf(2,"please give the path to find the file\n");   
        exit();
    }
    else if(argc==2){
        printf(2,"please give the file name \n");
        exit();
    }
    else if(argc==3){
        printf(1,"please give the file name after -name\n");
        exit();
    }
    else if(argc==4 && strcmp(argv[2],"-name")==0){                                    //for default find
        findname(argv[1],argv[3],0,"","");
    }
    else if(argc==5 && strcmp(argv[2],"-name")==0 &&strcmp(argv[4],"-printi")==0){    //for -printi flag
        findname(argv[1],argv[3],1,"","");
    }
    else if(argc==6 && strcmp(argv[2],"-name")==0 &&strcmp(argv[4],"-type")==0){     //for -type flag
        findname(argv[1],argv[3],0,argv[5],"");
    }
    else if(argc==6 && strcmp(argv[2],"-name")==0 &&strcmp(argv[4],"-inum")==0){     //for -inum flag
        findname(argv[1],argv[3],0,"",argv[5]);
    }
    exit();
}
