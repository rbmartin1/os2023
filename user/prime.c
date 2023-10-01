#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void child(int readFD)
{
  int prime;
  // nacita prve zapisane cislo a printne ho
  if (read(readFD, &prime, sizeof(prime)) == 0)
  {
    exit(1);
  }
  fprintf(0, "prime %d\n", prime);

  int outFD[2];
  if (pipe(outFD) < 0)
  {
    exit(1);
  }

  int pid = fork();

  if (pid > 0)
  {
    int num;
    close(outFD[0]);
    while (read(readFD, &num, sizeof(num)) > 0)
    {
      if (num % prime != 0)
      {
        if (write(outFD[1], &num, sizeof(num)) != sizeof(num))
        {
          exit(1);
        }
      }
    }
    close(outFD[1]);
    wait(0);
  }
  else
  {
    close(outFD[1]);
    child(outFD[0]);
  }
}

int main()
{
  int inOut[2];
  if (pipe(inOut) < 0)
  { 
    exit(1);
  }
  int pid = fork();
  int buf;
  if (pid > 0)
  {
    for (int i = 2; i <= 35; i++)
    {
      buf = i;
      close(inOut[0]);
      if (write(inOut[1], &buf, sizeof(buf)) != sizeof(buf))
      {
        exit(1);
      }
    }
    close(inOut[1]);
    wait(0);
  }
  else
  {
    close(inOut[1]);
    child(inOut[0]);   
  }
  exit(0);
}