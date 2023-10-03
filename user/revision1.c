#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


/*Fork and Print: Create a task where a parent process forks 
a child process. The child process should print a message, 
and the parent process should wait for the child to complete 
before printing its own message.
*/
/*
int
main(int argc, char *argv[])
{
    int pid = fork();

    if(pid == 0){
        printf("Child: Hello\n");
    }else if(pid > 0){
        wait(0);
        printf("Parent: World\n");
    }else{
        printf("Fork Error\n");
        exit(1);
    }


    exit(0);
}
*/



/*Inter-Process Communication: Develop a task where two processes 
communicate using a pipe. One process writes a message to the pipe, 
and the other process reads and prints the message. Ensure proper 
closing of file descriptors to avoid deadlocks.
*/
/*
int
main(int argc, char *argv[])
{
    char buff[5];
    int p[2];
    pipe(p);
    int pid = fork();

    if(pid == 0){
        write(p[1], "Hello", 5);
        close(p[0]);
        close(p[1]);
    }else if(pid > 0){
        close(p[1]);
        wait(0);
        read(p[0], buff, 5);
        close(p[1]);
        printf("from parent >> %s\n", buff);
    }else{
        printf("Error");
        exit(1);
    }

    exit(0);

}
*/


/*Process Tree: Create a program that generates a hierarchical 
process tree using multiple fork calls. Each process should print 
its own PID and the PID of its parent process.
*/

void proces(int actual, int max, int pid){
    printf("ch >> %d  p >> %d\n", getpid(), pid);

    pid = getpid();

    int next;

    if(actual < max){
        next = fork();
        if(next == 0){
            proces(actual + 1, max, pid);
            exit(0);
        }else{
            wait(0);
        }
    }
    
}


int
main(int argc, char *argv[])
{
    proces(0, 5, 1);

    exit(0);

}
