#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
  int pid = getpid();
  printf("Current process PID: %d\n", pid);
  exit(0);
}