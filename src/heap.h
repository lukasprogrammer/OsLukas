#ifndef HEAP_H
#define HEAP_H
#define KERNEL_HEAP_START 0xD0000000
#define HEAP_MAGIC 0xDEADBEEF

static unsigned int heap_next;
static unsigned int heap_mapped_end;

typedef struct HeapBlock {
    unsigned int size;
    int free;
    struct HeapBlock *next;
    unsigned int magic;
} HeapBlock;



void HeapInit(void);
void *kmalloc(unsigned int size);
void kfree(void *ptr);
void CoalesceFreeBlocks(void);
#endif 