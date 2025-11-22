#include "kernel/types.h"
#include "user/user.h"

int main() {
    int p1[2], p2[2];  // 两个管道
    char buf[1];
    int pid;
    
    // 创建两个管道
    pipe(p1);
    pipe(p2);
    
    pid = fork();
    if (pid == 0) {
        // 子进程
        close(p1[1]);  // 关闭父进程的写端
        close(p2[0]);  // 关闭父进程的读端
        
        // 从父进程读取数据
        read(p1[0], buf, 1);
        printf("%d: received ping\n", getpid());
        
        // 向父进程写入数据
        write(p2[1], "x", 1);
        
        close(p1[0]);
        close(p2[1]);
        exit(0);
    } else {
        // 父进程
        close(p1[0]);  // 关闭子进程的读端
        close(p2[1]);  // 关闭子进程的写端
        
        // 向子进程写入数据
        write(p1[1], "x", 1);
        
        // 从子进程读取数据
        read(p2[0], buf, 1);
        printf("%d: received pong\n", getpid());
        
        close(p1[1]);
        close(p2[0]);
        wait(0);
        exit(0);
    }
}
