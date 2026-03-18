// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct kmem{
  struct spinlock lock;
  struct run *freelist;
};

static struct kmem cpus_kmem[NCPU];

void
kinit()
{
  for(int i=0;i<NCPU;i++){
    char lockname[8];
    snprintf(lockname,sizeof(lockname),"kmem%d",i);
    initlock(&cpus_kmem[i].lock,lockname);
  }
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

// Free the page of physical memory pointed at by pa,
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
  push_off();
  int cid;
  cid = cpuid();
  pop_off();

  acquire(&cpus_kmem[cid].lock);
  r->next = cpus_kmem[cid].freelist;
  cpus_kmem[cid].freelist = r;
  release(&cpus_kmem[cid].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  r = 0;
  push_off();
  int cid;
  cid = cpuid();
  pop_off();
  acquire(&cpus_kmem[cid].lock);
  if(cpus_kmem[cid].freelist){
    r = cpus_kmem[cid].freelist;
    cpus_kmem[cid].freelist = r->next;
    release(&cpus_kmem[cid].lock);
  }
  else{
    release(&cpus_kmem[cid].lock);
    for(int i=0;i<NCPU;i++){
      if(cid == i)
	continue;
      acquire(&cpus_kmem[i].lock);
      if(cpus_kmem[i].freelist){
	r = cpus_kmem[i].freelist;
	cpus_kmem[i].freelist = r->next;
	release(&cpus_kmem[i].lock);
	break;
      }
      release(&cpus_kmem[i].lock);
    }
  }
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
