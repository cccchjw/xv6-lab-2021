#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "sysinfo.h"

int
main(int argc, char *argv[])
{
    // 检查参数
    if (argc != 1) {
        fprintf(2, "Usage: %s need not param\n", argv[0]);
        exit(1);
    }

    struct sysinfo info;
    
    // 调用系统调用获取系统信息
    if (sysinfo(&info) < 0) {
        fprintf(2, "sysinfo failed\n");
        exit(1);
    }
    
    // 打印系统信息
    printf("free space: %d bytes\n", info.freemem);
    printf("used process: %d\n", info.nproc);
    
    exit(0);
}
