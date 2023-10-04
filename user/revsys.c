#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
int
main(int argc, char *argv[])
{

    int a = revpid();
    printf("%d ", a);

    exit(0);
}
*/

int
main(int argc, char *argv[])
{
    char *message;

    for(int i = 0; i<sizeof(*argv);i++){
        message = argv[i];
        printsys(message);
    }

    exit(0);
}