#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"
//number of pointer slots in the command argument array
#define MAXARGS 16

int
main(int argc, char *argv[])
{
    //i is a loop position; rc receives the result of fork
    int i, rc;
    //holds the command and its arguments, followed by a null pointer
    char *newargv[MAXARGS];
    //the priority requested on the command line
    int priority;

    //we need at least the words pexec, priority and command
    if(argc < 3){
        //show how to use the command if any required words are missing
        printf("Usage: pexec <priority> <command> [args...]\n");
        //finish with an error status
        exit(-1);
    }

    //start building the priority number from its text digits
    priority = 0;
    //argv[1] is the priority text; visit each character until its ending zero
    for(i=0; argv[1][i]; i++){
        //reject non-digits and stop a number that is already too large from growing
        if(argv[1][i] < '0' || argv[1][i] > '9' || priority > 49){
            //write the error to file descriptor 2, the error output
            fprintf(2, "pexec: priority must be between 0 and 49\n");
            exit(-1);
        }
        //shift the number one decimal place and add the next digit
        //for example, reading 2 then 0 builds the number 20
        priority = priority * 10 + argv[1][i] - '0';
    }
    //reject empty input; otherwise ask the kernel to change this pexec process
    //setpriority returns -1 if the final number is outside 0 to 49
    if(argv[1][0] == 0 || setpriority(priority) < 0){
        fprintf(2, "pexec: priority must be between 0 and 49\n");
        exit(-1);
    }
    //skip pexec and the priority; copy the command and any remaining arguments
    for(i=2; i<argc; i++){
        //argv[2] becomes newargv[0], the command name, and so on
        newargv[i-2] = argv[i];
    }
    //a null pointer marks the end of the argument list passed to exec
    newargv[argc-2] = 0;

    //create a child that inherits this pexec process's new priority
    //fork returns the child's pid to the parent, 0 to the child, or -1 on failure
    rc = fork();
    //only the child gets zero, so only the child runs the next line
    if (rc == 0) {
        //replace the child's program with the requested command
        //the child keeps its pid and priority; successful exec does not return
        exec(newargv[0], newargv);
    }
    //the parent waits for a child to finish; 0 means ignore the exit status
    wait(0);
    //finish pexec successfully
    exit(0);
}
