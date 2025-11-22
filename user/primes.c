#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
prime_process(int pleft[2])
{
  int prime;
  int n;
  
  // 读取第一个数，它一定是素数
  if(read(pleft[0], &prime, sizeof(prime)) != sizeof(prime)){
    close(pleft[0]);
    exit(1);
  }
  
  printf("prime %d\n", prime);
  
  // 检查是否有更多数字
  if(read(pleft[0], &n, sizeof(n)) == 0){
    close(pleft[0]);
    exit(0);
  }
  
  int pright[2];
  pipe(pright);
  
  int pid = fork();
  if(pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }
  
  if(pid == 0){ // 子进程
    close(pright[1]);
    close(pleft[0]);
    prime_process(pright);
  } else { // 父进程
    close(pright[0]);
    
    // 过滤掉能被prime整除的数字
    do {
      if(n % prime != 0){
        write(pright[1], &n, sizeof(n));
      }
    } while(read(pleft[0], &n, sizeof(n)) > 0);
    
    close(pleft[0]);
    close(pright[1]);
    wait(0);
    exit(0);
  }
}

int
main(int argc, char *argv[])
{
  int p[2];
  pipe(p);
  
  int pid = fork();
  if(pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }
  
  if(pid == 0){ // 子进程
    close(p[1]);
    prime_process(p);
  } else { // 父进程
    close(p[0]);
    
    // 发送数字2到35
    for(int i = 2; i <= 35; i++){
      if(write(p[1], &i, sizeof(i)) != sizeof(i)){
        fprintf(2, "write failed\n");
        exit(1);
      }
    }
    
    close(p[1]);
    wait(0);
    exit(0);
  }
  
  return 0; // 不会执行到这里
}