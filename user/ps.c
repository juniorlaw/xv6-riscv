#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  //space for information about up to NPROC processes
  struct pstat uproc[NPROC];
  //number of processes returned by getprocs
  int nprocs;
  //position of the process we are printing
  int i;
  //current time in clock ticks
  uint currenttime;
  //points to the text describing one process state
  char *state;
  //state numbers are used as positions in this list
  static char *states[] = {
    "unused",
    "used",
    "sleeping",
    "runnable",
    "running",
    "zombie"
  };

  //ask the kernel to fill our array and return how many entries it filled
  nprocs = getprocs(uproc);
  //a negative result means the system call failed
  if (nprocs < 0) {
    //end ps with an error status
    exit(-1);
  }

  //read the clock after collecting readytime values so it is not older than them
  currenttime = uptime();

  //print the column names; each backslash-t creates a tab
  printf("pid\tstate\t\tsize\tage\tpriority\tcputime\tppid\tname\n");
  //visit each entry that the kernel filled
  for (i=0; i<nprocs; i++) {
    //turn the numeric state into a word such as sleeping or running
    state = states[uproc[i].state];
    //print this process's id, state and memory size in bytes
    printf("%d\t%s\t%ld\t", uproc[i].pid, state, uproc[i].size);
    //only runnable processes are waiting for a turn on the cpu
    if(uproc[i].state == RUNNABLE) {
      //age is the ticks since the process last became runnable
      printf("%u", currenttime - uproc[i].readytime);
    }
    //print priority, cpu ticks, parent id and name, then end the row
    //if the process is not runnable, the age column stays blank
    printf("\t%d\t\t%d\t%d\t%s\n",
          uproc[i].priority, uproc[i].cputime,
          uproc[i].ppid, uproc[i].name);
  }

  //finish ps successfully
  exit(0);
}
