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

struct {
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf buckets[NBUCKET];
  struct spinlock lks[NBUCKET]
} bcache;

static void
bufinit(struct buf* b, uint dev, uint blockno){
  b->dev = dev;
  b->blockno = blockno;
  b->valid = 0;
  b->refcnt = 1;
}

void
binit(void)
{
  struct buf *b;

  for(int i=0; i<NBUCKET; i++)
    initlock(&bcache.lks[i], "bcache");

  for(int i =0; i < NBUCKET; i++){
    // Create linked list of buffers
    bcache.buckets[i].prev = &bcache.buckets[i];
    bcache.buckets[i].next = &bcache.buckets[i];
  }

  

  
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.buckets[0].next;
    b->prev = &bcache.buckets[0];
    initsleeplock(&b->lock, "buffer");
    bcache.buckets[0].next->prev = b;
    bcache.buckets[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int id = myhash(blockno);

  acquire(&bcache.lks[id]);

  // Is the block already cached?
  for(b = bcache.buckets[id].next; b != &bcache.buckets[id]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lks[id]);
      acquiresleep(&b->lock);
      return b;
    }
  }


  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  struct buf *victim = 0;
  uint minticks = ticks;
  for(b = bcache.buckets[id].next; b != &bcache.buckets[id]; b = b->next){
    if(b->refcnt == 0 && b->lastuse <= minticks) {
      minticks = b.lastuse;
      victim = b;
    }
  }

  if(!victim)
    goto steal;

  bufinit(victim, dev, blockno);

  release(&bcache.lks[id]);
  acquiresleep(&victim->lock);
  return victim

steal:
    for(int i=0; i<NBUCKET; i++){
      if(i == id)
      continue;

    acquire(&bcache.lks[i]);
    minticks = ticks;
    for(b = bcache.buckets[i].next; b != &bcache.buckets[i]; b = b->next){
      if(b->refcnt==0 && b->lastuse <= minticks){
        minticks = b->lastuse;
        victim = b;
      }
    }

    if(!victim){
      release(&bcache.lks[i]);
      continue;
    }

    bufinit(victim, dev, blockno);

    victim->next->prev = victim->prev;
    victim->prev->next = victim->next;
    release(&bcache.lks[id]);

    victim->next = bcache.buckets[id].next;
    bcache.buckets[id].next->prev = victim;
    bcache.buckets[id].next = victim;
    victim->prev = &bcache.buckets[id];


    release(&bcache.lks[id]);
    acquiresleep(&victim->lock);
    return victim
  
    }
  release(&bcache.lks[id]);
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

  int id = myhash(b->blockno);
  acquire(&bcache.lks[id]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->lastuse = ticks;
  }
  
  release(&bcache.lks[id]);
}

void
bpin(struct buf *b) {
  int id = myhash(b->blockno);
  acquire(&bcache.lks[id]);
  b->refcnt++;
  release(&bcache.lks[id]);
}

void
bunpin(struct buf *b) {
  int id = myhash(b->blockno);
  acquire(&bcache.lks[id]);
  b->refcnt--;
  release(&bcache.lks[id]);
}

static int
myhash(int x){
  return x%NBUCKET;
}
