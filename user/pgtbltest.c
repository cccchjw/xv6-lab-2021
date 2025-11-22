#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

void ugetpid_test();
void pgaccess_test();
void superpg_test();

int
main(int argc, char *argv[])
{
  ugetpid_test();
  pgaccess_test();
  superpg_test();  // 添加这行
  printf("pgtbltest: all tests succeeded\n");
  exit(0);
}

char *testname = "???";

void
err(char *why)
{
  printf("pgtbltest: %s failed: %s, pid=%d\n", testname, why, getpid());
  exit(1);
}

void
ugetpid_test()
{
  int i;

  printf("ugetpid_test starting\n");
  testname = "ugetpid_test";

  for (i = 0; i < 64; i++) {
    int ret = fork();
    if (ret != 0) {
      wait(&ret);
      if (ret != 0)
        exit(1);
      continue;
    }
    if (getpid() != ugetpid())
      err("missmatched PID");
    exit(0);
  }
  printf("ugetpid_test: OK\n");
}

void
pgaccess_test()
{
  char *buf;
  unsigned int abits;
  printf("pgaccess_test starting\n");
  testname = "pgaccess_test";
  buf = malloc(32 * PGSIZE);
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
  buf[PGSIZE * 1] += 1;
  buf[PGSIZE * 2] += 1;
  buf[PGSIZE * 30] += 1;
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
  if (abits != ((1 << 1) | (1 << 2) | (1 << 30)))
    err("incorrect access bits set");
  free(buf);
  printf("pgaccess_test: OK\n");
}


void superpg_test()
{
    printf("superpg_test starting\n");
    testname = "superpg_test";
    
    // 分配 2MB 内存
    char *buf = sbrk(2*1024*1024);
    if(buf == (char*)-1)
        err("sbrk failed");
    
    // 测试内存访问
    for(int i = 0; i < 2*1024*1024; i += PGSIZE) {
        buf[i] = i & 0xFF;
    }
    
    // 验证
    for(int i = 0; i < 2*1024*1024; i += PGSIZE) {
        if(buf[i] != (i & 0xFF))
            err("incorrect data");
    }
    
    printf("superpg_test: OK\n");
}

