#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

#define CHILDREN 3

struct result {
  int pid;
  int priority;
  uint response;
  uint turnaround;
  int cputime;
};

static void
check(int ok)
{
  if(!ok){
    fprintf(2, "agingtest: failed\n");
    exit(1);
  }
}

static int
cpuused(void)
{
  //keep the process array off xv6's small user stack
  static struct pstat procs[NPROC];
  int n = getprocs(procs);
  for(int i=0; i<n; i++)
    if(procs[i].pid == getpid())
      return procs[i].cputime;
  check(0);
  return 0;
}

static void
average(char *name, uint total)
{
  uint fraction = (total % CHILDREN) * 100 / CHILDREN;
  printf("average %s: %u.", name, total / CHILDREN);
  if(fraction < 10)
    printf("0");
  printf("%u ticks\n", fraction);
}

int
main(void)
{
  int priorities[CHILDREN] = {0, 20, 40};
  int budgets[CHILDREN] = {8, 35, 65};
  int gate[2], ready[2], done[2];
  int savedpriority = getpriority();
  check(pipe(gate)==0 && pipe(ready)==0 && pipe(done)==0);
  //the parent releases all workers before letting any of them run
  check(setpriority(49)==0);

  for(int i=0; i<CHILDREN; i++){
    int pid = fork();
    check(pid >= 0);
    if(pid == 0){
      close(gate[1]);
      close(ready[0]);
      close(done[0]);
      struct result r;
      r.pid = getpid();
      r.priority = priorities[i];
      check(setpriority(r.priority)==0);
      check(write(ready[1], "r", 1)==1);

      uint arrival;
      check(read(gate[0], &arrival, sizeof(arrival))==sizeof(arrival));
      //first user-space timestamp after the release, just after dispatch
      r.response = (uint)uptime() - arrival;
      int startcpu = cpuused();
      while(cpuused() - startcpu < budgets[i])
        for(volatile int k=0; k<10000; k++)
          ;
      //completion means the end of the CPU workload, before reporting/exit
      r.turnaround = (uint)uptime() - arrival;
      r.cputime = cpuused() - startcpu;
      check(write(done[1], &r, sizeof(r))==sizeof(r));
      exit(0);
    }
  }
  close(gate[0]);
  close(ready[1]);
  close(done[1]);
  char token;
  for(int i=0; i<CHILDREN; i++)
    check(read(ready[0], &token, 1)==1);
  //give the last worker time to block on the gate after announcing readiness
  check(pause(1)==0);
  uint arrivals[CHILDREN];
  uint arrival = uptime();
  for(int i=0; i<CHILDREN; i++)
    arrivals[i] = arrival;
  check(write(gate[1], arrivals, sizeof(arrivals))==sizeof(arrivals));
  close(gate[1]);
  close(ready[0]);

  struct result results[CHILDREN];
  uint response = 0, turnaround = 0;
  for(int i=0; i<CHILDREN; i++){
    check(read(done[0], &results[i], sizeof(results[i]))==sizeof(results[i]));
    response += results[i].response;
    turnaround += results[i].turnaround;
  }
  close(done[0]);
  for(int i=0; i<CHILDREN; i++){
    int status;
    check(wait(&status)>0 && status==0);
  }
  check(setpriority(savedpriority)==0);

#ifdef AGING_ENABLED
  printf("agingtest: aging enabled\n");
#else
  printf("agingtest: aging disabled\n");
#endif
  printf("pid\tpriority\tresponse\tturnaround\tcpu ticks\n");
  for(int i=0; i<CHILDREN; i++)
    printf("%d\t%d\t\t%u\t\t%u\t\t%d\n", results[i].pid,
           results[i].priority, results[i].response, results[i].turnaround,
           results[i].cputime);
  average("response", response);
  average("turnaround", turnaround);
  exit(0);
}
