#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p1[2], p2[2];
  char buf[1];
  
  // 创建两个管道
  if(pipe(p1) < 0 || pipe(p2) < 0){
    fprintf(2, "pipe failed\n");
    exit(1);
  }
  
  int pid = fork();
  if(pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }
  
  if(pid == 0){ // 子进程
    close(p1[1]); // 关闭父进程的写端
    close(p2[0]); // 关闭子进程的读端
    
    // 从父进程读取字节
    if(read(p1[0], buf, 1) != 1){
      fprintf(2, "child read failed\n");
      exit(1);
    }
    close(p1[0]);
    
    printf("%d: received ping\n", getpid());
    
    // 写回给父进程
    if(write(p2[1], buf, 1) != 1){
      fprintf(2, "child write failed\n");
      exit(1);
    }
    close(p2[1]);
    
    exit(0);
  } else { // 父进程
    close(p1[0]); // 关闭父进程的读端
    close(p2[1]); // 关闭子进程的写端
    
    buf[0] = 'x';
    // 发送字节给子进程
    if(write(p1[1], buf, 1) != 1){
      fprintf(2, "parent write failed\n");
      exit(1);
    }
    close(p1[1]);
    
    // 从子进程读取字节
    if(read(p2[0], buf, 1) != 1){
      fprintf(2, "parent read failed\n");
      exit(1);
    }
    close(p2[0]);
    
    printf("%d: received pong\n", getpid());
    
    wait(0);
    exit(0);
  }
}