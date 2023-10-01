#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int pid = fork();
    int p1[2];
    int p2[2];
    pipe(p1);
    pipe(p2);
    char recv_buf[5];

    if (pid == 0){
    write(p1[1],"1", 5);
    close(p1[0]);
    close(p2[1]);
    read(p2[0],recv_buf, 5);
    printf("%d: received ping\n", getpid());
  } else {
    write(p2[1],"2", 5);
    close(p2[1]);
    close(p1[1]);
    wait(0);
    read(p1[0],recv_buf, 5);
    printf("%d: received pong\n", getpid());
  }

    exit(0);
}

