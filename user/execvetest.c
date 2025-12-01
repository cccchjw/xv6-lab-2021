#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    printf("=== Simple Environment Test ===\n");
    
    char *argv[] = {
        "echo",
        "test",
        0
    };
    
    char *envp[] = {
        "TEST=simple",
        0
    };
    
    printf("Calling execve with simple environment...\n");
    int result = execve("echo", argv, envp);
    
    printf("execve failed: %d\n", result);
    exit(1);
}