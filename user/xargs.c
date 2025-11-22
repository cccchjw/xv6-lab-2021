#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"  // 包含 MAXARG 定义

int main(int argc, char *argv[]) {
    // 参数检查
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }

    char buf[1024];        // 读取缓冲区
    char *args[MAXARG];    // 参数数组
    char c;
    int n = 0;            // 缓冲区索引
    int i, pid;
    
    // 复制原始参数（跳过 xargs 本身）
    for (i = 0; i < argc - 1; i++) {
        args[i] = argv[i + 1];
    }

    // 读取标准输入
    while (read(0, &c, 1) > 0) {
        if (c == '\n') {
            // 遇到换行符，处理当前行
            if (n > 0) {
                buf[n] = 0;  // 字符串结束符
                
                // 添加当前行作为最后一个参数
                args[argc - 1] = buf;
                args[argc] = 0;  // 参数数组以0结尾
                
                // 创建子进程执行命令
                pid = fork();
                if (pid == 0) {
                    // 子进程执行命令
                    exec(args[0], args);
                    // 如果 exec 失败
                    fprintf(2, "xargs: exec %s failed\n", args[0]);
                    exit(1);
                } else {
                    // 父进程等待子进程完成
                    wait(0);
                }
                
                // 重置缓冲区索引
                n = 0;
            }
        } else {
            // 将字符添加到缓冲区
            if (n < sizeof(buf) - 1) {
                buf[n++] = c;
            }
            // 如果缓冲区满，忽略剩余字符
        }
    }

    // 处理最后一行（如果没有换行符结尾）
    if (n > 0) {
        buf[n] = 0;
        args[argc - 1] = buf;
        args[argc] = 0;
        
        pid = fork();
        if (pid == 0) {
            exec(args[0], args);
            fprintf(2, "xargs: exec %s failed\n", args[0]);
            exit(1);
        } else {
            wait(0);
        }
    }

    exit(0);
}
