Siddhartha Reddy Guthikonda - U62740661
Chakradhar Reddy Nallu-U22128895
Deepthi Muttineni - U88032559

ticks_running():

--> In proc.h i have included a variable "ticks_running" in struct proc for tracking num of ticks.
--> In proc.c the ticks_running is initialized to 0 in allocproc() function (it begins from zero for each new process)
--> ticks_running is incremented everytime a process is scheduled and running , this is done in scheduler() func in proc.c
 and also implemented the sys_ticks_running() system call,this is called in sysproc.c
-->This  system call function  sys_ticks_running, which, given a process ID, returns the total number of ticks the process has ran.
It first retrieves the process ID argument and acquires a lock on the process table.Then, it iterates through the process table to find the process with the given ID. 
It locates the process with the specified ID by iteratively searching the process table;after processing it releases the process table lock
 if the process cannot be located, it returns -1.
--> in syscall.h we will newly declare the system call for ticks_running as 22 based on my system.
--> and also in syscall.c i have added a handler for this function.
--> Updated the user-space library in usys.S and user.h to include a ticks_running() wrapper func.
-->finally in the user space implemenation created a ticks_running.c which takes single arg (pid) the process ID implicitly using getpid() for the process that is running on the system
and gives output based on the no of ticks it is running.  returns the time scheduled for the calling process.
returns the time scheduled for any process id, returns 0 for unscheduled process , returns -1 for  process that does not exist.


Simple scheduler:
we have chosed FIFO SCHEDULER.
###FIFO scheduler 
--we have implemented fifo scheduler inside the proc.c file. the fifo scheduler is called at the implementation time by cflags which are mentioned in makefile.
--it iterates over the each process in the process table and selectcts the process which is arrived first and completes its execution.
-- we also have incremented the ticks_runs inside the scheduler.

-fifo_position() function gives the process's position in  the FIFO queue.
- It enters a loop to create n child processes using fork() system call.
- Each of the child process Prints its PID and start message.
- Prints the process position in the FIFO queue using a fifo_position() function 
- Each child process sleeps for 100 ticks before finishing using sleep(100)
- If fork() function fails then an error message is printed, and the program exits.
- The parent process waits for all child processes to complete before printing a final completion message and exiting.

#Testing Fifo:
-Testing:
-we have created a file named simple_Scheduler_test for testing the implemented FIFO-scheduler. 
-We have created 5 process and for each process we have called fork() func inside the for loop
-when the child process is created  it sets the priorities  by calling the system call and gets its pid's from the getpid() sys call.The process starts their execution according to their priorities.
-If the pid < 0 then it prints as process failed.
-The parents waits till the child process gets terminated.


Advanced_scheduler:
we have implemented multi-level priority scheduling and two system calls named set_sched_priority() and get_sched_priority().
We used three levels [low, medium, high] and declared them in proc.h
we assigned the default priority of "Medium" for every new process created in the fork() method in proc.c
Then we implemented the priority scheduling in proc.c inside the priority cflag.
Implementation of priority scheduling:
 we implemented priority scheduling in the scheduler function
 we acquired the ptable lock using acquire(&ptable) then we iterated over the ptable using for loop.
 In that loop, during the scheduling cycle, the process which is in runnable("RUNNABLE") state is selected with the highest priority["HIGH"].
 process execution: the selected process is set to running ["RUNNING"] state. and CPU context is switched for its execution.
 we have initialized the start time.
 Then we calculated ticks_Running by incrementing every time the process is scheduled.

Testing:
we have created a file named advanced_Scheduler_test for testing the implemented priority scheduler. 
We have created 3 process and for each process we have called fork() function inside the for loop
when the child process is created. It sets the priorities  by calling the system call  set_sched_priority(priority) and gets its pid's from the getpid() sys call. We also used get_sched_priority() system call for getting the priority of current running process.The process starts their execution according to their priorities inside the developed priority scheduler code.
If the pid < 0 then it prints as process failed.
The parents waits till the child process gets terminated.




