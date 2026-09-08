#include "memory.h"
#include "paging.h"
#include "terminal.h"
static unsigned int *page_directory;
static unsigned int page_directory_phys;

void PagingInit(void)
{
    page_directory_phys = AllocPage();
    page_directory = (unsigned int *)page_directory_phys;

    for (int i = 0; i < PAGE_ENTRIES; i++) {
        page_directory[i] = 0;
    }


    unsigned int identity_tables =
        (128 * 1024 * 1024) / (PAGE_ENTRIES * PAGE_SIZE);

    for (unsigned int directory = 0;
         directory < identity_tables;
         directory++) {

        unsigned int table_phys = AllocPage();

        unsigned int *table =
            (unsigned int *)table_phys;


        for (unsigned int t = 0; t < PAGE_ENTRIES; t++) {

            unsigned int physical_address =
                (directory * PAGE_ENTRIES + t) * PAGE_SIZE;

            table[t] =
                physical_address |
                PAGE_PRESENT |
                PAGE_WRITE;
        }


        page_directory[directory] =
            table_phys |
            PAGE_PRESENT |
            PAGE_WRITE;
    }

    LoadPageDirectory(page_directory_phys);
    EnablePaging();

}
void MapPage(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags){
    unsigned int table_index = (virtual_addr >> 12) & 0x3FF;
    unsigned int directory_index = virtual_addr >> 22;
    unsigned int *table;

    if(page_directory[directory_index] & PAGE_PRESENT){
        unsigned int table_phys = page_directory[directory_index] & 0xFFFFF000;
        table = (unsigned int *)table_phys;

        if(flags & PAGE_USER){
            page_directory[directory_index] |= PAGE_USER;
        }
        
    }else{
        unsigned int table_phys = AllocPage();
        table = (unsigned int *)table_phys;

        for(int i = 0; i < PAGE_ENTRIES; i++){
            table[i] = 0;
        }

        unsigned int directory_flags = PAGE_PRESENT | PAGE_WRITE;
        if(flags & PAGE_USER){
            directory_flags |= PAGE_USER;
        }
        page_directory[directory_index] = table_phys | directory_flags;
    }

    table[table_index] = (physical_addr & 0xFFFFF000) | flags |PAGE_PRESENT;
    InvalidatePage(virtual_addr);
}

int UnmapPage(unsigned int virtual_addr){
    unsigned int table_index = (virtual_addr >> 12) & 0x3FF;
    unsigned int directory_index = virtual_addr >> 22;
    
    
    if (!(page_directory[directory_index] & PAGE_PRESENT)) {
    return 0;
    }
    unsigned int table_phys = page_directory[directory_index] & 0xFFFFF000;
    unsigned int *table = (unsigned int *)table_phys;

    if (!(table[table_index] & PAGE_PRESENT)) {
    return 0;
    }

    table[table_index] = 0;
    InvalidatePage(virtual_addr);
    return 1;

}
void UnmapPageInDirectory(unsigned int directory_phys, unsigned int virtual_addr){
    unsigned int *directory = (unsigned int *)directory_phys;
    unsigned int table_index = (virtual_addr >> 12) & 0x3FF;
    unsigned int directory_index = virtual_addr >> 22;
    
    
    if (!(directory[directory_index] & PAGE_PRESENT)) {
    return ;
    }
    unsigned int table_phys = directory[directory_index] & 0xFFFFF000;
    unsigned int *table = (unsigned int *)table_phys;

    if (!(table[table_index] & PAGE_PRESENT)) {
    return ;
    }

    table[table_index] = 0;
    return;

}

void FreePageTableInDirectory(
    unsigned int directory_phys,
    unsigned int directory_index)
{
    if (directory_index >= 1024) {
        return;
    }

    unsigned int *directory =
        (unsigned int *)directory_phys;

    if (!(directory[directory_index] & PAGE_PRESENT)) {
        return;
    }

    unsigned int table_phys =
        directory[directory_index] & 0xFFFFF000;

    directory[directory_index] = 0;

    FreePage(table_phys);
}
unsigned int GetPhysicalAddress(unsigned int virtual_addr){
    unsigned int table_index = (virtual_addr >> 12) & 0x3FF;
    unsigned int directory_index = virtual_addr >> 22;
    unsigned int offset = virtual_addr & 0xFFF;
    if (!(page_directory[directory_index] & PAGE_PRESENT)) {
    return 0;
    }
    unsigned int table_phys = page_directory[directory_index] & 0xFFFFF000;
    unsigned int *table = (unsigned int *)table_phys;

    if (!(table[table_index] & PAGE_PRESENT)) {
    return 0;
    }

    return (table[table_index] & 0xFFFFF000) + offset;

}
unsigned int GetPhysicalAddressInDirectory(unsigned int directory_phys, unsigned int virtual_addr){
    unsigned int *directory =
        (unsigned int *)directory_phys;

    unsigned int table_index = (virtual_addr >> 12) & 0x3FF;
    unsigned int directory_index = virtual_addr >> 22;
    unsigned int offset = virtual_addr & 0xFFF;
    if (!(directory[directory_index] & PAGE_PRESENT)) {
        return 0;
    }
    unsigned int table_phys = directory[directory_index] & 0xFFFFF000;
    unsigned int *table = (unsigned int *)table_phys;

    if (!(table[table_index] & PAGE_PRESENT)) {
    return 0;
    }

    return (table[table_index] & 0xFFFFF000) + offset;
}   



unsigned int CreateKernelStack(void){
    unsigned int stack_bottom = KERNEL_STACK_TOP - KERNEL_STACK_PAGES*PAGE_SIZE;
    for(int i = 0; i < KERNEL_STACK_PAGES; i++){
        unsigned int phys = AllocPage();
        if (phys == 0) {
            return 0;
        }
        unsigned int virtual = stack_bottom + i*PAGE_SIZE;

        MapPage(virtual, phys, PAGE_WRITE);
    }
    return KERNEL_STACK_TOP;
}

int IsUserAddress(unsigned int virtual_addr)
{
    unsigned int directory_phys;

    asm volatile(
        "mov %%cr3, %0"
        : "=r"(directory_phys)
    );

    unsigned int *directory =
        (unsigned int *)directory_phys;

    unsigned int directory_index =
        virtual_addr >> 22;

    unsigned int table_index =
        (virtual_addr >> 12) & 0x3FF;

    if (!(directory[directory_index] & PAGE_PRESENT)) {
        return 0;
    }

    if (!(directory[directory_index] & PAGE_USER)) {
        return 0;
    }

    unsigned int table_phys =
        directory[directory_index] & 0xFFFFF000;

    unsigned int *table =
        (unsigned int *)table_phys;

    unsigned int entry =
        table[table_index];

    if (!(entry & PAGE_PRESENT)) {
        return 0;
    }

    if (!(entry & PAGE_USER)) {
        return 0;
    }

    return 1;
}


unsigned int GetKernelPageDirectory(void){
    return (unsigned int)page_directory;
}
unsigned int CreateUserPageDirectory(void){
    unsigned int new_pd_phys = AllocPage();

    if(new_pd_phys == 0){
        return 0;
    }

    unsigned int *new_pd = (unsigned int *)new_pd_phys;

    for(unsigned int i = 0; i < 1024; i++){
        new_pd[i] = page_directory[i];
    }

    new_pd[256] = 0;
    new_pd[511] = 0;
    new_pd[512] = 0;

    // for (unsigned int i = 0; i < 256; i++) {
    //     new_pd[i] = page_directory[i];
    // }

    return new_pd_phys;
}

void MapPageInDirectory(unsigned int directory_phys,unsigned int virtual_addr,unsigned int physical_addr,unsigned int flags){

    unsigned int *directory = (unsigned int *)directory_phys;
    unsigned int directory_index = virtual_addr >> 22;
    unsigned int table_index = (virtual_addr >> 12) & 0x3FF;

    unsigned int *table;

    if(directory[directory_index] && PAGE_PRESENT){
        unsigned int table_phys = directory[directory_index] & 0xFFFFF000;

        table = (unsigned int *)table_phys;
    }else {
        unsigned int table_phys = AllocPage();

        if(table_phys == 0){
            return;
        }
        
        table = (unsigned int *)table_phys;

        for (unsigned int i = 0; i < 1024; i++) {
            table[i] = 0;
        }

        unsigned int directory_flags =
            PAGE_PRESENT | PAGE_WRITE;

        if (flags & PAGE_USER) {
            directory_flags |= PAGE_USER;
        }

        directory[directory_index] =
            table_phys | directory_flags;
    }

    table[table_index] =
        (physical_addr & 0xFFFFF000)
        | flags
        | PAGE_PRESENT;

}
void SwitchPageDirectory(unsigned int directory_phys)
{
    asm volatile(
        "mov %0, %%cr3"
        :
        : "r"(directory_phys)
        : "memory"
    );
}