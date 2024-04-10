#include "types.h"
#include "user.h"
#include "stat.h"


int main(int argc, char *argv[])
{
    if(argc < 2){
        printf(2, "usage: give the number of seconds the process should pause\n");
        exit();
    }
    if(argv[1][0]=='-'){
        printf(2,"Error: time cannot be negative %s\n",argv[1]);
    }

    int ticks = atoi(argv[1]);  //atoi function is used to convert string arguments to integer arguments
    if(ticks<0){
        printf(2,"Error: time cannot be negative %s\n",argv[1]);
        exit();
    }
    sleep(ticks);  // pauses for given number of ticks in command line arguments
    exit();
}