#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int start, end;
  int pid;

  // A command must be provided.
  if(argc < 2){
    fprintf(2, "usage: time1 command [args...]\n");
    exit(1);
  }

  // Get the starting time before running the command.
  start = uptime();

  pid = fork();

  if(pid < 0){
    fprintf(2, "time1: fork failed\n");
    exit(1);
  }

  // Child process runs the requested command.
  if(pid == 0){
    exec(argv[1], &argv[1]);

    // exec only returns if it failed.
    fprintf(2, "time1: exec %s failed\n", argv[1]);
    exit(1);
  }

  // Parent waits for the child to finish.
  wait(0);

  // Get the ending time and calculate elapsed time.
  end = uptime();

  printf("elapsed time: %d ticks\n", end - start);

  exit(0);
}