#include "types.h"
#include "stat.h"
#include "user.h"
#define HIGH   0
#define MEDIUM 1
#define LOW    2
void computation(int count) {
    while (count > 0) {
        int i,x=0;
        for (i = 0; i < 10000; i++){
            x*=10;
        }
        count-=1;
    }
}
int main(void) {
    int pid, i,k;
    int priorities[] = {HIGH, MEDIUM, LOW};
    for(i = 0; i < 3; i++) {
        pid = fork();
        if(pid == 0) {//child process is created
            set_sched_priority(priorities[i]);  //calling the created child process and setting its priority respectively by using the developed system call.
            k=get_sched_priority(getpid());
            printf(1, " The Child process %d: PID %d has started its execution with priority %d\n", i, getpid(),k);
            sleep(100); // sleeps for 100 ticks
            printf(1, "The Child process %d: PID %d has finished its execution with priority %d\n", i, getpid(),k);
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
    printf(1, "The priority  scheduler test completed\n");
    exit();
}