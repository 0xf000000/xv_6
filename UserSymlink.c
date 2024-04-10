#include "types.h"
#include "stat.h"
#include "user.h"


// usermode programm implementation for symlink
// note i did it like this but u can easily make a cli programm out of this
int main(int argc,char** argv){
        char buf[20];
        int fd = open("./input.txt", 0);
        read(fd, (char*) buf, sizeof(buf));

        printf(1, " %s\n", buf);
        close(fd);

        symlink("./input.txt", "john/hello.txt");

        int fd2 = open("john/hello.txt",0);

        
        
        memset((char*) &buf, 0, sizeof(buf));
        read(fd2, (char*) buf, sizeof(buf) );

        printf(1, " %s\n", buf);


        exit();

}