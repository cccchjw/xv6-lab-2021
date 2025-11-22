#include "kernel/types.h"
#include "user/user.h"

// 声明 primes 函数不会返回，避免编译器警告
void primes(int read_fd) __attribute__((noreturn));

void primes(int read_fd) {
    int prime, n;
    
    // 读取第一个数，它一定是素数
    if (read(read_fd, &prime, sizeof(prime)) == 0) {
        close(read_fd);
        exit(0);
    }
    
    // 打印素数
    printf("prime %d\n", prime);
    
    // 创建新管道用于与下一级进程通信
    int p[2];
    pipe(p);
    
    if (fork() == 0) {
        // 子进程：继续筛选下一级素数
        close(p[1]);  // 关闭写端
        primes(p[0]); // 递归处理
        exit(0);      // 实际上不会执行到这里，因为 primes 被声明为 noreturn
    } else {
        // 父进程：过滤当前素数的倍数
        close(p[0]);  // 关闭读端
        
        // 读取剩余数字并过滤倍数
        while (read(read_fd, &n, sizeof(n)) > 0) {
            if (n % prime != 0) {
                write(p[1], &n, sizeof(n));
            }
        }
        
        // 关闭管道和文件描述符
        close(read_fd);
        close(p[1]);
        
        // 等待子进程完成
        wait(0);
        exit(0);
    }
}

int main() {
    int p[2];
    pipe(p);
    
    if (fork() == 0) {
        // 子进程：开始素数筛
        close(p[1]);  // 关闭写端
        primes(p[0]); // 从管道读取数字
        exit(0);
    } else {
        // 父进程：生成数字 2 到 35
        close(p[0]);  // 关闭读端
        
        for (int i = 2; i <= 35; i++) {
            write(p[1], &i, sizeof(i));
        }
        
        close(p[1]);  // 关闭写端，表示输入结束
        wait(0);      // 等待所有子进程完成
        exit(0);
    }
}
