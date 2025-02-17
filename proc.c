#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

int str_cmp(const char *p, const char *q)
{
  while (*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);
extern int sys_uptime(void);
static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
  acquire(&tickslock);
  p->creation_time = ticks;
  p->wait_time=0;
  release(&tickslock);

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;
  if(np->priority!=LOW && np->priority!=MEDIUM &&np->priority!=HIGH){
    np->priority=MEDIUM;
  }
  // np->creation_time = ticks;
  acquire(&ptable.lock);

  
  np->start_time=ticks;

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }
  acquire(&tickslock);
  curproc->end_time=ticks;
  release(&tickslock);
  curproc->wait_time=curproc->start_time-curproc->creation_time;
  curproc->turnaround_time = curproc->end_time - curproc->creation_time;
  // if (str_cmp(curproc->name,"sh")!=0 ){
  // cprintf("NAME\t\tPID\tCreation Time\tstart time\tEnd Time\tWait Time\tTurnAround Time\t\t uptime\t \n");

  // cprintf("%s\t%d \t \t%d\t\t%d\t \t%d \t\t%d\t \t%d \t\t%d\t \n",curproc->name,curproc->pid,curproc->creation_time,curproc->start_time,curproc->end_time,curproc->wait_time,curproc->turnaround_time,sys_uptime());
  // }
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

//#ifdef DEFAULT
void scheduler(void)
{
    struct proc *p;
    struct cpu *c = mycpu();
    c->proc = 0;
    
    for(;;){
      // Enable interrupts on this processor.
      sti();
      // Loop over process table looking for process to run.
      acquire(&ptable.lock);
      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE)
          continue;
        // Switch to chosen process.  It is the process's job
        // to release ptable.lock and then reacquire it
        // before jumping back to us.
        c->proc = p;
        switchuvm(p);
        p->state = RUNNING;
        
        swtch(&(c->scheduler), p->context);
        p->ticks_runs++;
        switchkvm();
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
      }
      release(&ptable.lock);
    }
}
//#endif
#ifdef FIFO
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    sti();//allows interrupts
    acquire(&ptable.lock);
    struct proc *selected = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    }
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;
      if(selected == 0 || p->creation_time < selected->creation_time){//selects the process which has less creation time
          selected = p;
        }
    }
    if (selected != 0) {
      c->proc = selected;
      switchuvm(selected);
      selected->state = RUNNING;
      swtch(&(c->scheduler), selected->context);
      selected->ticks_runs++;
      switchkvm();
      c->proc = 0;
    }
    release(&ptable.lock);
  }
}
#endif

#ifdef PRIORITY
void scheduler(void) {
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  for (;;) {
    sti();
    acquire(&ptable.lock); // Acquire the process table lock before making changes
    struct proc *selected = 0; // setting the selected proc =0 for each scheduling cycle
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->state != RUNNABLE)
        continue;
      // Correctly use an if statement to update selected based on priority
      if (!selected || selected->priority > p->priority) {
        selected = p;//highest process is selected
      }
    }
    if (selected) {
      p = selected; // This is the process to run
      c->proc = p;
      switchuvm(p);
      if(p->start_time==0){//if start time is 0, it updated te start_Time
          acquire(&tickslock);
          p->start_time = ticks; // Set start time to current ticks
          release(&tickslock);
      }
      // cprintf("pid %dpriority %d is running first \n",p->pid,p->priority);
      p->state = RUNNING;
      swtch(&(c->scheduler), p->context);
      p->ticks_runs++;//incrementes the ticks
      switchkvm();
      c->proc = 0;
    }
    release(&ptable.lock); // Release the process table lock after scheduling is done
  }
}
#endif


// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// Set the current process's priority
int
set_sched_priority(int priority)
{
  if(priority < 0 || priority > NPRIORITIES-1) //checks the priority should not be <0 and >2
    return -1;
  myproc()->priority = priority;//assigns the priority to the current process
  // cprintf("current prioriyt is %d",myproc()->priority);
  return 0;
}
// Get the priority of the process with given pid
int
get_sched_priority(int pid)
{
  struct proc *p;  //initialized the process pointer
  acquire(&ptable.lock);//acquires the lock of ptable
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){ //iterates throug p table
    if(p->pid == pid){//if process id matches
      if(p->state == RUNNABLE || p->state == RUNNING ){
          release(&ptable.lock);//releases the ptable lock
          // cprintf("print priority %d",p->priority);
          return p->priority;//returns the priority of process
        }
      }
  }
  release(&ptable.lock);
  return -1; // returns -1 if the process is not in the process table and not in execution
}


int fifo_position(int pid) {
  int position = 0;//index position variable
  struct proc *p;// creating struct proc
  int found = 0;// creating found variable

  acquire(&ptable.lock); // aquiring lock
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state == RUNNABLE || p->state == RUNNING || p->state == SLEEPING) {
      if(p->pid == pid) {//if pid matches it executes the code
        found = 1;
        break;
      }
      position++;// incrementing the position
    }
  }
  release(&ptable.lock);//releasing the lock

  if(found) return position;  //returns given position
  else return -1; // returns -1 if pid is not found
}

int
ticks_running(int pid)
{
  int tar_pid, acc_ticks;
  struct proc *curr_proc;

  if(argint(0, &tar_pid) < 0) //if fetching is failed
    return -1;

  acquire(&ptable.lock);//aquiring the ptable lock

  for(curr_proc = ptable.proc; curr_proc < &ptable.proc[NPROC]; curr_proc++) {//iterating through ptable
    if(curr_proc->pid == tar_pid) {//if the current pid matches with target pid
    if(curr_proc->state == UNUSED){// if process state is unused
      acc_ticks=-1;
    }
    else{// if process state other than unused
      acc_ticks=curr_proc->ticks_runs;
    }
    release(&ptable.lock); 
    return acc_ticks;//returning ticks
    }
  }

  release(&ptable.lock); 
  return -1; 
}

// allocate space for clone and pass it into same adress space
int clone(void(*fcn)(void*), void *arg, void* stack){

    int i, pid;
    struct proc *newProcc;
     struct proc* currentProcc = myproc();
    // well something really bad has to happen in order for this to execute but better having a checkup
     if( currentProcc == 0){
       panic(" clone: cant return current procces\n");
       }

    int* ustack = stack + PGSIZE - 4;

  
    if( (newProcc = allocproc()) == 0){
      return -1;
    }

    newProcc->pgdir = currentProcc->pgdir;
    newProcc->sz = currentProcc->sz;
    
    newProcc->ustack = stack;
    newProcc->parent = 0;
    *newProcc->tf = *currentProcc->tf;


    newProcc->tf->eax = newProcc->pid;
    newProcc->tf->ebp = (int) ustack - 4;
    newProcc->tf->esp = (int) ustack - 4;
    newProcc->tf->eip = (int) fcn; 

    *ustack = (int) arg;


    *(ustack -1) = 0xffffffff;
    *(ustack -2) = 0xffffffff;

    for(int i = 0; i < NOFILE; i++){
      if(currentProcc->ofile[i]){
        newProcc->ofile[i] = filedup(newProcc->ofile[i]);
      }

      newProcc->cwd = idup(currentProcc->ofile[i]);

    }
    



}
