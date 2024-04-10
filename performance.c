#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int pid =0;
    if (strcmp(argv[1],"cat")==0){
        if (pid == 0){
        char *value[]={"uniq","input.txt",0};
        exec(value[0],value);
    }
    }
    if (strcmp(argv[1],"find")==0){
        char *value[]={argv[1],argv[2],argv[3],argv[4],0};
        if (pid == 0){
            exec(value[0],value);
        }
    }
    if (pid == 0){
        char *value[]={argv[1],0};
        exec(value[0],value);
    }
}