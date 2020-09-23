#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void childProcess(int parent[2]){

  int firstNum; // prime to be printed
  int testNum; // following number to pipe
  int n; // return value of read()
  
  close(parent[1]); // close parent write end for return value = 0

  n = read(parent[0], &firstNum, 4);
  printf("prime %d\n", firstNum);
  n = read(parent[0], &testNum, 4);
  if (n == 0) { // do not fork() and pipe()
    // printf("n = 0, exit");
    exit(0);
  } else { // create child process
    int newp[2];
    pipe(newp); // create pipe between parent and child
    if (fork() == 0) // child process
    {
      childProcess(newp);
    } else {
      close(newp[0]); // close read end of self
      // before if (n == 0); we got testNum
      // check prime
      if (testNum % firstNum != 0) {
        write(newp[1], &testNum, 4);
        // 可注释
        // printf(" write pipe following number %d\n", testNum);
      }
      for (;;) {
        n = read(parent[0], &testNum, 4); // read from parent
        if (n == 0) break;
        if (n < 0) {
          fprintf(2, "read from left neighbor error\n");
        } else {
          // check prime
          if (testNum % firstNum != 0) {
            write(newp[1], &testNum, 4); // write to child
            // printf(" write pipe following number %d\n", testNum);
          }
        }
      }
      // close child pipe write end
      close(newp[1]);
      wait((int *) 0);
    }
  }
  // close parent read end
  close(parent[0]);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int p[2];
  pipe(p);
  if (fork() == 0) { // child process
    childProcess(p);
  } else { // parent process
      close(p[0]);
      printf("prime 2\n");
      for (int i = 2; i <= 35; i++)
      {
        if ((i & 1) != 0) {
          write(p[1], &i, 4);
        }
      }
    
    
    close(p[1]);
    wait((int *) 0);
  }
  exit(0);
}

