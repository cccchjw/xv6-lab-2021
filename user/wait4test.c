#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    printf("=== Testing wait4 time statistics ===\n");
    
    // 测试1：快速进程
    printf("\nTest 1: Quick process (should have minimal time)\n");
    int pid1 = fork();
    if(pid1 == 0) {
        // 快速退出
        exit(1);
    }
    
    int status1;
    struct rusage ru1;
    wait4(pid1, &status1, 0, &ru1);
    printf("Quick process: user=%lu, sys=%lu, mem=%lu pages\n", 
           ru1.ru_utime, ru1.ru_stime, ru1.ru_maxrss);
    
    // 测试2：中等工作负载
    printf("\nTest 2: Medium workload\n");
    int pid2 = fork();
    if(pid2 == 0) {
        // 做一些工作
        volatile int sum = 0;
        for(int i = 0; i < 100000; i++) {
            sum += i;
        }
        exit(2);
    }
    
    struct rusage ru2;
    wait4(pid2, &status1, 0, &ru2);
    printf("Medium workload: user=%lu, sys=%lu, mem=%lu pages\n", 
           ru2.ru_utime, ru2.ru_stime, ru2.ru_maxrss);
    
    // 测试3：分配内存的进程
    printf("\nTest 3: Process with memory allocation\n");
    int pid3 = fork();
    if(pid3 == 0) {
        // 分配一些内存
        char *mem = malloc(4096 * 20);  // 20页
        for(int i = 0; i < 4096 * 20; i++) {
            mem[i] = i % 256;
        }
        free(mem);
        exit(3);
    }
    
    struct rusage ru3;
    wait4(pid3, &status1, 0, &ru3);
    printf("Memory process: user=%lu, sys=%lu, mem=%lu pages\n", 
           ru3.ru_utime, ru3.ru_stime, ru3.ru_maxrss);
    
    printf("\n=== Time statistics test completed ===\n");
    exit(0);
}