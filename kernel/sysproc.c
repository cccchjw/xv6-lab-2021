#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;
  
  addr = p->sz;
  
  // 如果分配大小 >= 2MB 且地址 2MB 对齐，尝试使用超级页
  if(n >= 2*1024*1024 && (addr % (2*1024*1024) == 0)) {
    // 尝试分配超级页
    if(superalloc() != 0) {
      // 设置超级页映射
      if(mappages(p->pagetable, addr, 2*1024*1024, 
                  (uint64)superalloc(), PTE_W|PTE_X|PTE_R|PTE_U) >= 0) {
        p->sz += 2*1024*1024;
        return addr;
      } else {
        superfree(superalloc());
      }
    }
  }
  
  // 回退到普通分配
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


#ifdef LAB_PGTBL
uint64
sys_kpgtbl(void)
{
  vmprint(myproc()->pagetable);
  return 0;
}
#endif




#ifdef LAB_PGTBL
uint64
sys_pgaccess(void)
{
  // 获取参数
  uint64 base;  // 起始虚拟地址
  int len;      // 页面数量
  uint64 user_buf; // 用户空间的结果缓冲区
  
  if(argaddr(0, &base) < 0 || 
     argint(1, &len) < 0 || 
     argaddr(2, &user_buf) < 0) {
    return -1;
  }
  
  // 参数检查
  if(len < 0 || len > 64) {  // 限制最大页面数
    return -1;
  }
  
  struct proc *p = myproc();
  pagetable_t pagetable = p->pagetable;
  
  // 计算位掩码
  unsigned int abits = 0;
  
  for(int i = 0; i < len; i++) {
    uint64 va = base + i * PGSIZE;
    pte_t *pte = walk(pagetable, va, 0);
    
    if(pte && (*pte & PTE_V) && (*pte & PTE_U)) {
      if(*pte & PTE_A) {
        // 设置对应的位
        abits |= (1 << i);
        // 清除访问位
        *pte &= ~PTE_A;
      }
    }
  }
  
  // 将结果复制回用户空间
  if(copyout(pagetable, user_buf, (char *)&abits, sizeof(abits)) < 0) {
    return -1;
  }
  
  return 0;
}
#endif