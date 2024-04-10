// In testsbrk.c

#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int old_size, new_size;

    old_size = (int)sbrk(0); // Get the initial process memory size

    printf(1, "Initial process memory size: %d\n", old_size);

    // Increase the process memory size by 100 bytes
    sbrk(100);
    new_size = (int)sbrk(0); // Get the new process memory size
    printf(1, "Process memory size after increasing by 100 bytes: %d\n", new_size);

    // Increase the process memory size by 500 bytes
    sbrk(500);
    new_size = (int)sbrk(0); // Get the new process memory size
    printf(1, "Process memory size after increasing by 500 bytes: %d\n", new_size);

    // Increase the process memory size by -200 bytes (decreasing)
    sbrk(-200);
    new_size = (int)sbrk(0); // Get the new process memory size
    printf(1, "Process memory size after decreasing by 200 bytes: %d\n", new_size);

    exit();
}
