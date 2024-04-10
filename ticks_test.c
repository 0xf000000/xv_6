#include "types.h"
#include "user.h"
#include "stat.h"

int main(int argc, char *argv[]) {
  int pid;
  
  // Optionally, get pid from command line arguments
  if(argc > 1) {
    pid = atoi(argv[1]);
  } else {
    pid = getpid(); // Use the current process's pid
  }

  int ticks = ticks_running(pid);
  printf(1, "Process %d has been running for %d ticks.\n", pid, ticks);

  exit();
}
