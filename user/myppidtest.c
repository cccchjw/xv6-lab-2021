#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
  int pid = getpid();
  int ppid = getppid();
  
  printf("Current process PID: %d\n", pid);
  printf("Parent process PID: %d\n", ppid);
  
  // 测试fork后的父子关系
  int ret = fork();
  if(ret == 0) {
    // 子进程
    printf("Child: my PID=%d, parent PID=%d\n", getpid(), getppid());
    exit(0);
  } else {
    // 父进程
    wait(0);  // 等待子进程退出
    printf("Parent: my PID=%d, parent PID=%d\n", getpid(), getppid());
  }
  
  exit(0);
}