#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{   int p_id = getpid(); //implicitly taking the pid here
    
    int numticks = ticks_running(p_id);
    
    if(numticks == -1)
        printf(2, "ticks for Process with PID %d does not existis %d\n", p_id,numticks);
    
    else if(numticks == 0)
        printf(1, "ticks for Process with PID %d is currently unscheduled is %d\n", p_id,numticks);
    
    else
        printf(1, "Ticks running for process %d: %d\n", p_id, numticks);
    
    exit();
}