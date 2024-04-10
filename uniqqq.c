/*#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

#define MAXCHARS 1000
#define MAX_Rows 14


int main(int argc, char *argv[]) {
  int fd, n, i;
  char buf[MAXCHARS], *p, *q, *last;
  if (argc != 3) {
    printf(1, "usage: uniq -w <filename>\n");
    exit();
  }
  if ((fd = open(argv[2], 0)) < 0) {
    printf(1, "uniq: cannot open %s\n", argv[2]);
    exit();
  }
  last = 0;
  n = atoi(argv[1]);
  if (n <= 0) {
    printf(1, "uniq: invalid width\n");
    exit();
  }
  while ((i = read(fd, buf, sizeof(buf))) > 0) {
    p = buf;
    while (p < buf + i) {
      q = p;
      while (*q && *q != '\n')
        q++;
      if (*q == '\n')
        *q++ = 0;
      int cmp = 1;
      if (last) {
        cmp = 0;
        for (int j = 0; j < n; j++) {
          if (last[j] != p[j]) {
            cmp = 1;
            break;
          }
        }
      }
      if (cmp) {
        printf(1, "%s\n", p);
        last = p;
      }
      p = q;
    }
  }
  close(fd);
  exit();
}
*/