#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    int pid, i, pos;
    printf(1, "Starting FIFO scheduler test with 5 processes\n");
    for(i = 0; i < 5; i++) {
        pid = fork();
        if(pid == 0) {//child process is created
            printf(1, "The Child process %d: PID %d started its execution\n", i, getpid());
            pos = fifo_position(getpid());//calling fifo_position system call
            printf(1, "The Child process %d: PID %d is at the position of %d in the implemented FIFO queue\n", i, getpid(), pos);
            sleep(100); // sleeps for 100 ticks
            printf(1, "The Child process %d: PID %d finished\n", i, getpid());
            sleep(100);//sleeps for 100 ticks
            exit();
        } else if(pid < 0) {// if creation of process fails
            printf(2, "calling Fork process failed\n");
            exit();
        }
    }
    for(i = 0; i < 5; i++) {
        wait();//parent process is waiting for teh child process to complete its execution
    }
    printf(1, "FIFO scheduler test completed\n");
    exit();
}