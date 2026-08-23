#include "heap.h"
#include "memory.h"
#include "paging.h"
#include "terminal.h"

static HeapBlock *heap_first = (HeapBlock *)0;
static HeapBlock *heap_last = (HeapBlock *)0;



void HeapInit(void){
    heap_next = KERNEL_HEAP_START;


    unsigned int phys = AllocPage();
    if (phys == 0) {
        return;
    }
    heap_mapped_end = KERNEL_HEAP_START + PAGE_SIZE;

    MapPage(heap_next, phys, PAGE_WRITE);
        

}

void *kmalloc(unsigned int size){

    unsigned int aligned_size = (size + 7) & ~7;
    unsigned int total_size = sizeof(HeapBlock) + aligned_size;
    HeapBlock *current = heap_first;
    while (current != (HeapBlock *)0) {

        if(current->free && current->size >= aligned_size){
            unsigned int remaining = current->size - aligned_size;
            if (remaining >= sizeof(HeapBlock) + 8) {
                unsigned char *headerPos = ((unsigned char *)(current + 1)) + aligned_size;
                HeapBlock *new_block = (HeapBlock *)headerPos;
                new_block->size = remaining - sizeof(HeapBlock);
                new_block->free = 1;
                new_block->next = current->next;
                new_block->magic = HEAP_MAGIC;
                current->next = new_block;
                current->size = aligned_size;
                if (heap_last == current) {
                    heap_last = new_block;
                }
            }
            current->free = 0;
            return (void *)(current + 1);
        }

        current = current->next;
    }

    


    if(heap_next + total_size > heap_mapped_end){
        while(heap_next + total_size > heap_mapped_end){
            unsigned int phys = AllocPage();
            if (phys == 0) {
                return (void *)0;
            }
            MapPage(heap_mapped_end, phys, PAGE_WRITE);
            heap_mapped_end += PAGE_SIZE;
            
        }

    }


    HeapBlock *block = (HeapBlock *)heap_next;
    block->size = aligned_size;
    block->free = 0;
    block->next = (HeapBlock *)0;
    block->magic = HEAP_MAGIC;
    if (heap_first == (HeapBlock *)0) {
        heap_first = block;
        heap_last = block;
    } else {
        heap_last->next = block;
        heap_last = block;  
    }
    
    heap_next += total_size;

    return (void *)(block + 1);
}
void kfree(void *ptr)
{
    if (ptr == (void *)0) {
        return;
    }

    HeapBlock *block = ((HeapBlock *)ptr) - 1;
    if (block->magic != HEAP_MAGIC) {
        return;
    }
    if (block->free) {
    return;
    }
    block->free = 1;
    CoalesceFreeBlocks();



}

void CoalesceFreeBlocks(void){
    HeapBlock *current = heap_first;
    while(current != 0 && current->next != 0){
        if (current->free && current->next->free) {
            current->size += current->next->size + sizeof(HeapBlock);
            current->next = current->next->next;
            

            if (current->next == 0) {
                heap_last = current;
            }
        }else{
            current = current->next;
        }
    }
}