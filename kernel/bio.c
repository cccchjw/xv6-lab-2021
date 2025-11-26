// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13  // 哈希桶数量，使用质数减少冲突

struct {
  struct spinlock lock[NBUCKET];  // 每个桶一个锁
  struct buf buf[NBUF];
  
  // 每个哈希桶的链表头
  struct buf head[NBUCKET];
} bcache;

void
binit(void)
{
  struct buf *b;

  // 初始化每个哈希桶的锁和链表
  for (int i = 0; i < NBUCKET; i++) {
    initlock(&bcache.lock[i], "bcache");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }

  // 将所有缓冲区分配到哈希桶中
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    // 使用缓冲区索引作为简单的哈希，均匀分布
    int bucket = (b - bcache.buf) % NBUCKET;
    
    b->next = bcache.head[bucket].next;
    b->prev = &bcache.head[bucket];
    initsleeplock(&b->lock, "buffer");
    bcache.head[bucket].next->prev = b;
    bcache.head[bucket].next = b;
  }
}

// 哈希函数
static int
hash(uint dev, uint blockno)
{
  return blockno % NBUCKET;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int target_bucket = hash(dev, blockno);
  
  acquire(&bcache.lock[target_bucket]);

  // Is the block already cached?
  for(b = bcache.head[target_bucket].next; b != &bcache.head[target_bucket]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[target_bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached in target bucket.
  // Recycle the least recently used (LRU) unused buffer in target bucket.
  for(b = bcache.head[target_bucket].prev; b != &bcache.head[target_bucket]; b = b->prev){
    if(b->refcnt == 0) {
      // Found unused buffer in target bucket
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[target_bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // No unused buffer in target bucket, need to steal from other buckets
  release(&bcache.lock[target_bucket]);
  
  // Try to steal from other buckets
  for (int i = 0; i < NBUCKET; i++) {
    if (i == target_bucket) continue;
    
    acquire(&bcache.lock[i]);
    
    // Look for unused buffer in this bucket
    for(b = bcache.head[i].prev; b != &bcache.head[i]; b = b->prev){
      if(b->refcnt == 0) {
        // Found unused buffer, steal it
        // Remove from current bucket
        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.lock[i]);
        
        // Add to target bucket
        acquire(&bcache.lock[target_bucket]);
        b->next = bcache.head[target_bucket].next;
        b->prev = &bcache.head[target_bucket];
        bcache.head[target_bucket].next->prev = b;
        bcache.head[target_bucket].next = b;
        
        // Initialize the stolen buffer
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        
        release(&bcache.lock[target_bucket]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.lock[i]);
  }
  
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int bucket = hash(b->dev, b->blockno);
  acquire(&bcache.lock[bucket]);
  b->refcnt--;
  
  if (b->refcnt == 0) {
    // Move to head of bucket's MRU list
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[bucket].next;
    b->prev = &bcache.head[bucket];
    bcache.head[bucket].next->prev = b;
    bcache.head[bucket].next = b;
  }
  
  release(&bcache.lock[bucket]);
}

void
bpin(struct buf *b) {
  int bucket = hash(b->dev, b->blockno);
  acquire(&bcache.lock[bucket]);
  b->refcnt++;
  release(&bcache.lock[bucket]);
}

void
bunpin(struct buf *b) {
  int bucket = hash(b->dev, b->blockno);
  acquire(&bcache.lock[bucket]);
  b->refcnt--;
  release(&bcache.lock[bucket]);
}