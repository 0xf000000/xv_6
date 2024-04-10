
#include "types.h"
#include "stat.h"
#include "user.h"

// lseek USERMODE implementation Part 4 Task 1 

int main(int argc, char** argv){
   int charsWeSkipp = 5;
   char buf[5];   
   int fd = open("./input.txt", 0 );
   read(fd, (char*) &buf, sizeof( buf));

   printf(1, " in the buffer are the first %d chars of the text: %s  \n", charsWeSkipp, buf);

   lseek(fd, 5); // the cool systcall for this task

   memset((char*) &buf,0, sizeof(buf));

   read(fd, (char*) &buf, sizeof( buf));

   printf(1, " %d character got skipped and we have now these chars: %s\n", charsWeSkipp, buf);


   close(fd);

     

   exit();
}