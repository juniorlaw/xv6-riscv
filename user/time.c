#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int start, end;
  int pid;
  int status;
  int elapsed;
  int percent;
  struct rusage usage;

  //make sure a command was provided
  if(argc < 2){
    fprintf(2, "usage: time command [args...]\n");
    exit(1);
  }

  //get the starting wall clock time
  start = uptime();

  pid = fork();

  if(pid < 0){
    fprintf(2, "time: fork failed\n");
    exit(1);
  }

  //run the requested command in the child
  if(pid == 0){
    exec(argv[1], &argv[1]);

    //exec only returns if it fails
    fprintf(2, "time: exec %s failed\n", argv[1]);
    exit(1);
  }

  //wait for the child and get its cpu usage
  if(wait2(&status, &usage) < 0){
    fprintf(2, "time: wait2 failed\n");
    exit(1);
  }

  //calculate elapsed wall clock time
  end = uptime();
  elapsed = end - start;

  //calculate cpu percentage
  if(elapsed > 0)
    percent = (usage.cputime * 100) / elapsed;
  else
    percent = 0;

  printf("elapsed time: %d ticks, cpu time: %d ticks, %d%% CPU\n",
         elapsed, usage.cputime, percent);

  exit(0);
}