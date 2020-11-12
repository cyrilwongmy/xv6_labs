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
void freepage(char * pa);
void increase_ref(void* pa, int exist);
int decrease_ref(void* pa);
int pa2idx(void *pa);
void* allocpage(void);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  int ref_count[1<<15]; // ref count for each page
  int used; // 不知道什么用？
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  memset(&kmem.ref_count, 0, sizeof(kmem.ref_count));
  kmem.used = 0;
}

// just use to free pages in kinit time.
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    // kfree(p);
    freepage(p);
}


// caller should hold lock
// Free a pa and add back to freelisk
void
freepage(char * pa) {
  memset(pa, 1, PGSIZE);
  struct run* r = (struct run*)pa;
  r->next = kmem.freelist;
  kmem.freelist = r;
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  // struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  if (decrease_ref(pa)) // only free pa when ref == 0
  {
    kmem.used--;
    freepage(pa);
  }
  
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  acquire(&kmem.lock);
  void* page = allocpage();

  if(page) {
    increase_ref(page, 0);
  }

  release(&kmem.lock);
  return page;
}

int
decrease_ref(void* pa) {
  int idx = pa2idx(pa);
  if (kmem.ref_count[idx] <= 0)
  {
    printf("%p should exist", pa);
    panic("decrease_ref");
  }
  return --kmem.ref_count[idx] == 0;
}

void 
increase_ref(void* pa, int exist) {
  int idx = pa2idx(pa);
  if(exist && kmem.ref_count[idx] <= 0) { // <= 优先级更高
    printf("page %p should exist", pa);
    panic("increase ref");
  } else if(!exist && kmem.ref_count[idx] > 0) {
    printf("page %p should not exist", pa);
    panic("increase ref");
  }
  kmem.ref_count[idx]++;
}

int
pa2idx(void *pa) {
  return (pa - (void *)end) / PGSIZE;
}

void*
allocpage()
{
  struct run* r = kmem.freelist ? kmem.freelist : 0;
  if(r) {
    kmem.used++;
    kmem.freelist = r->next;
    memset((char*)r, 5, PGSIZE);
  }
  return r;
}