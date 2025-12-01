#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  int mask;
  if(argint(0, &mask) < 0)
    return -1;
  
  myproc()->systrace = mask;
  return 0;
}



uint64
sys_sysinfo(void)
{
  // addr is a user virtual address, pointing to a struct sysinfo
  uint64 addr;
  struct sysinfo info;
  struct proc *p = myproc();
  
  if (argaddr(0, &addr) < 0)
	  return -1;
  // get the number of bytes of free memory
  info.freemem = free_mem();
  // get the number of processes whose state is not UNUSED
  info.nproc = nproc();

  if (copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
  
  return 0;
}

uint64
sys_getppid(void)
{
  struct proc *p = myproc();
  
  // 检查父进程是否存在且有效
  if(p->parent && p->parent != p) {
    return p->parent->pid;
  }
  
  // 如果没有父进程（如init进程），返回1（init的PID）
  return 1;
}

uint64
sys_clone(void)
{
  uint64 flags;
  uint64 stack;
  
  if(argaddr(0, &flags) < 0 || argaddr(1, &stack) < 0)
    return -1;
    
  return clone(flags, (void*)stack);
}

uint64
sys_execve(void)
{
  char path[MAXPATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;
  int ret = -1;  // 初始化 ret 为 -1

  if(argstr(0, path, MAXPATH) < 0 || argaddr(1, &uargv) < 0) {
    return -1;
  }
  
  // 重用 sys_exec 的参数处理逻辑
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0){
      goto bad;
    }
    if(fetchstr(uarg, argv[i], PGSIZE) < 0){
      goto bad;
    }
  }

  ret = exec(path, argv);

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return ret;
}
