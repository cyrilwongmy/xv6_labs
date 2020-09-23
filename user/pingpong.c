#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2];
  char buf[8];
  int n;

  pipe(p);

  // create child process
  if(fork() == 0) { // child process
    // receive a byte from parent
    for (;;) {
      n = read(p[0], buf, sizeof buf);
      if (n == 0)
        break;
      if (n < 0) {
        fprintf(2, "read from parent error\n");
        exit(1);
      } else {
        printf("%d: received ping\n", getpid());
        // write the byte back to parent
        write(p[1], buf, sizeof buf);
        break;
      }
    }
    exit(0);
     
  } else { // parent process
    // parent send a byte to the child
    write(p[1], "i", 1);
    wait((int *) 0);
    // receive the byte from the child
    n = read(p[0], buf, sizeof buf);
    if (n > 0) {
      printf("%d: received pong\n", getpid());
      close(p[0]);
      close(p[1]);
    }
    exit(0);
  }
}
