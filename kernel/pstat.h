#ifndef PSTAT_H
#define PSTAT_H

//define the possible states of a process
enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct rusage {
  uint cputime;
};

struct pstat {
  int pid;
  enum procstate state;
  uint64 size;
  int ppid;
  char name[16];
};

#endif