// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}


// 分配 2MB 对齐的物理内存
void*
superalloc(void)
{
  acquire(&kmem.lock);
  
  // 寻找 2MB 对齐的连续 2MB 内存块
  struct run *p;
  for(p = kmem.freelist; p; p = p->next) {
    // 检查是否 2MB 对齐
    if((uint64)p % (2*1024*1024) == 0) {
      // 检查是否有足够的连续页面
      int found = 1;
      struct run *current = p;
      for(int i = 0; i < 512; i++) { // 2MB = 512 * 4KB
        if(!current || (uint64)current % PGSIZE != 0) {
          found = 0;
          break;
        }
        current = current->next;
      }
      
      if(found) {
        // 从空闲列表中移除这些页面
        struct run *result = p;
        struct run *prev = 0;
        current = kmem.freelist;
        
        // 找到 p 在链表中的前一个节点
        while(current && current != p) {
          prev = current;
          current = current->next;
        }
        
        // 从链表中移除 512 个页面
        if(prev) {
          prev->next = current;
          for(int i = 0; i < 512 && current; i++) {
            current = current->next;
          }
          prev->next = current;
        } else {
          // p 是链表头
          kmem.freelist = current;
          for(int i = 0; i < 512 && kmem.freelist; i++) {
            kmem.freelist = kmem.freelist->next;
          }
        }
        
        release(&kmem.lock);
        return (void*)result;
      }
    }
  }
  
  release(&kmem.lock);
  return 0;
}

// 释放超级页
void
superfree(void *pa)
{
  if((uint64)pa % (2*1024*1024) != 0)
    panic("superfree: not 2MB aligned");
  
  acquire(&kmem.lock);
  
  // 将 512 个页面添加回空闲列表
  struct run *first = (struct run*)pa;
  struct run *current = first;
  
  for(int i = 1; i < 512; i++) {
    struct run *next = (struct run*)((char*)pa + i * PGSIZE);
    current->next = next;
    current = next;
  }
  current->next = kmem.freelist;
  kmem.freelist = first;
  
  release(&kmem.lock);
}