#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fcntl.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

  argint(0, &pid);
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
sys_mmap(void)
{
  uint64 addr, length, offset;
  int prot, flags, fd;
  struct file *f;
  struct proc *p = myproc();
  struct vma *v;

  argaddr(0, &addr);
  argaddr(1, &length);
  argint(2, &prot);
  argint(3, &flags);
  argint(4, &fd);
  argaddr(5, &offset);

  if(length == 0)
    return -1;
  if(offset % PGSIZE != 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f = p->ofile[fd]) == 0)
    return -1;
  if((prot & PROT_WRITE) && (flags & MAP_SHARED) && !f->writable)
    return -1;

  v = 0;
  for(int i = 0; i < NVMA; i++){
    if(!p->vmas[i].used){
      v = &p->vmas[i];
      break;
    }
  }
  if(v == 0)
    return -1;

  addr = p->sz;
  length = PGROUNDUP(length);
  p->sz += length;

  v->used = 1;
  v->addr = addr;
  v->len = length;
  v->prot = prot;
  v->flags = flags;
  v->f = filedup(f);
  v->offset = offset;

  return addr;
}

uint64
sys_munmap(void)
{
  uint64 addr, length;
  struct proc *p = myproc();
  struct vma *v;
  pte_t *pte;

  argaddr(0, &addr);
  argaddr(1, &length);

  if(addr % PGSIZE != 0)
    return -1;

  length = PGROUNDUP(length);

  for(int i = 0; i < NVMA; i++){
    v = &p->vmas[i];
    if(v->used && addr >= v->addr && addr + length <= v->addr + v->len){
      for(uint64 a = addr; a < addr + length; a += PGSIZE){
        pte = walk(p->pagetable, a, 0);
        if(pte && (*pte & PTE_V)){
          if(v->flags & MAP_SHARED){
            uint64 off = v->offset + (a - v->addr);
            uint64 n = PGSIZE;
            if(off < v->f->ip->size){
              if(off + n > v->f->ip->size)
                n = v->f->ip->size - off;
              begin_op();
              writei(v->f->ip, 1, a, off, n);
              end_op();
            }
          }
          uvmunmap(p->pagetable, a, 1, 1);
        }
      }
      if(addr == v->addr && addr + length == v->addr + v->len){
        v->used = 0;
        fileclose(v->f);
      } else if(addr == v->addr){
        v->addr += length;
        v->offset += length;
        v->len -= length;
      } else if(addr + length == v->addr + v->len){
        v->len -= length;
      }
      return 0;
    }
  }
  return -1;
}
