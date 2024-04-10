#include "types.h"
#include "user.h"

#define PAGE_SIZE 4096  

int main(void) {
    int *p1;
    int i;

    // Here we allocating memory using sbrk()

    p1= (int*)sbrk(PAGE_SIZE);  // allocation of one page

    if ((int)p1 == -1) {
        printf(1,"sbrk() failed to allocate the memory\n");
        exit();
    }

    // Writing the data to the allocated memory
    for (i = 0; i < 8*PAGE_SIZE / sizeof(int); i++) {
        p1[i] = i;
    }

    // here we are reading the the allocated memory and printing
    for (i = 0; i < PAGE_SIZE / sizeof(int); i++) {
        printf(1,"%d ", p1[i]);
    }
    printf(1,"\n");

    exit();
}
