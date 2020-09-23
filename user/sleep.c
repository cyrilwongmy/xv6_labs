// include
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[])
{
  int seconds;
  // arugment check
  if(argc < 2){
    fprintf(2, "Usage: sleep seconds ...\n");
    exit(1);
  }

  // call system call sleep
  // convert arg string to integer
  seconds = atoi(argv[0]);
  sleep(seconds);

  // exit
  exit(0);

}
