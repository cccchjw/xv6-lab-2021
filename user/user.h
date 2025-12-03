struct stat;
struct rtcdate;
struct sysinfo;
struct rusage {
  uint64 ru_utime;    // 用户态运行时间（时钟滴答）
  uint64 ru_stime;    // 内核态运行时间（时钟滴答）
  uint64 ru_maxrss;   // 最大驻留集大小（页面数）
  uint64 ru_ixrss;    // 共享内存大小
  uint64 ru_idrss;    // 非共享数据大小
  uint64 ru_isrss;    // 非共享栈大小
  uint64 ru_minflt;   // 缺页次数（不需要I/O）
  uint64 ru_majflt;   // 缺页次数（需要I/O）
  uint64 ru_nswap;    // 交换次数
  uint64 ru_inblock;  // 块输入操作
  uint64 ru_oublock;  // 块输出操作
  uint64 ru_msgsnd;   // 发送的消息数
  uint64 ru_msgrcv;   // 接收的消息数
  uint64 ru_nsignals; // 接收的信号数
  uint64 ru_nvcsw;    // 自愿上下文切换
  uint64 ru_nivcsw;   // 非自愿上下文切换
};


// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int trace(int);
struct sysinfo;
int sysinfo(struct sysinfo *);
int getppid(void);
int clone(uint64 flags, void *stack);
int execve(const char *path, char *argv[], char *envp[]);
int wait4(int pid, int *status, int options, void *rusage);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
