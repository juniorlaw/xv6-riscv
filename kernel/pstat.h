#ifndef PSTAT_H
#define PSTAT_H

//define the possible states of a process
enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

//store the cpu time used by a process
struct rusage {
  uint cputime;
};

//store process information for the ps command
struct pstat {
  int pid;               //process id
  enum procstate state;  //current process state
  uint64 size;           //process memory size in bytes
  int ppid;              //parent process id
  int priority;          //process priority from 0 to 49
  uint readytime;        //time when the process last became runnable
  int cputime;           //total cpu time used by the process
  char name[16];         //process name
};

#endif