#include "types.h"
#include "stat.h"
#include "user.h"


#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_NOFOLLOW 0x04 //syslink

#define BSIZE 512

int main(int argc, char** argv){

    char block [BSIZE];
    int fd = open("input.txt", O_RDWR);

    for(int i = 0; i < BSIZE; i++){
        block[i] = 's';
    }

    for( int j = 0; j < 16523; j++){
        
        printf(1," writing %d\n", j);
        int data =   write(fd, (char*) &block, sizeof(block));
        if(data < 0 ){
            printf(1, "this is fucked\n");
        }
    }
    char buf[10];

    read(fd, (char*) buf, sizeof(buf) );

    printf(1,"%s\n", buf);

    close(fd);
exit();


    return 0;
}