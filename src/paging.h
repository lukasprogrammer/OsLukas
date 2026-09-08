#ifndef PAGING_H
#define PAGING_H
#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2
#define PAGE_USER    0x4

#define PAGE_ENTRIES 1024
#define IDENTITY_TABLES (MAX_MEMORY / (PAGE_ENTRIES * PAGE_SIZE))

#define KERNEL_STACK_TOP   0xC0400000
#define KERNEL_STACK_PAGES 4

void PagingInit(void);
void MapPage(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags);
unsigned int GetPhysicalAddress(unsigned int virtual_addr);
int UnmapPage(unsigned int virtual_addr);
unsigned int CreateKernelStack(void);
int IsUserAddress(unsigned int virtual_addr);
unsigned int GetKernelPageDirectory(void);
unsigned int CreateUserPageDirectory(void);
void SwitchPageDirectory(unsigned int directory_phys);

unsigned int GetPhysicalAddressInDirectory(
    unsigned int directory_phys,
    unsigned int virtual_addr
);

void UnmapPageInDirectory(
    unsigned int directory_phys,
    unsigned int virtual_addr
);

void MapPageInDirectory(
    unsigned int directory_phys,
    unsigned int virtual_addr,
    unsigned int physical_addr,
    unsigned int flags
);

void FreePageTableInDirectory(
    unsigned int directory_phys,
    unsigned int directory_index
);


static inline void InvalidatePage(unsigned int virtual_addr)
{
    __asm__ volatile(
        "invlpg (%0)"
        :
        : "r"(virtual_addr)
        : "memory"
    );
}

static inline void LoadPageDirectory(unsigned int addr)
{
    __asm__ volatile(
        "mov %0, %%cr3"
        :
        : "r"(addr)
        : "memory"
    );
}

static inline void EnablePaging(void)
{
    unsigned int cr0;

    __asm__ volatile(
        "mov %%cr0, %0"
        : "=r"(cr0)
    );

    cr0 |= 0x80000000;

    __asm__ volatile(
        "mov %0, %%cr0"
        :
        : "r"(cr0)
        : "memory"
    );
}

#endif