#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("\n");
  printf("Bread'OS v1.0 - Available Commands:\n");
  printf("====================================\n");
  printf("cat    - display file content\n");
  printf("echo   - display a line of text\n");
  printf("ls     - list directory contents\n");
  printf("mkdir  - make directories\n");
  printf("rm     - remove files or directories\n");
  printf("kill   - terminate a process\n");
  printf("sleep  - sleep for specified ticks\n");
  printf("find   - search for files\n");
  printf("xargs  - build and execute command lines\n");
  printf("primes - generate prime numbers\n");
  printf("trace  - trace system calls\n");
  printf("help   - show this help message\n");
  printf("\n");
  printf("For more information about a command, please wait\n");
  printf("\n");
  
  exit(0);
}