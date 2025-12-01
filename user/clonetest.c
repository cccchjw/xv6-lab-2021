#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define STACK_SIZE 4096

// 简单的 clone 测试程序
int main() {
    printf("Testing clone system call...\n");
    
    // 为子进程分配栈空间
    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        printf("Failed to allocate stack\n");
        exit(1);
    }
    
    // 栈通常从高地址向低地址增长
    char *stack_top = stack + STACK_SIZE;
    
    printf("Parent PID: %d\n", getpid());
    printf("Allocated stack: %p to %p\n", stack, stack_top);
    
    int pid = clone(0, stack_top);
    
    if (pid == 0) {
        // 子进程
        printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
        printf("Child: using custom stack\n");
        free(stack);
        exit(0);
    } else if (pid > 0) {
        // 父进程
        int status;
        wait(&status);
        printf("Parent: child %d exited with status %d\n", pid, status);
        free(stack);
    } else {
        printf("Clone failed!\n");
        free(stack);
        exit(1);
    }
    
    printf("Clone test completed successfully\n");
    exit(0);
}