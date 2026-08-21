#include "memory.h"
#include "terminal.h"

#define MEMORY_MAP_COUNT  ((volatile unsigned short *)0x4FF0)
#define MEMORY_MAP_BUFFER ((volatile struct memory_map_entry *)0x5000)

#define PAGE_SIZE       0x1000
#define MAX_MEMORY      (128 * 1024 * 1024)
#define MAX_PAGES       (MAX_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE     (MAX_PAGES / 8)

static unsigned char page_bitmap[BITMAP_SIZE];
static unsigned char reserved_bitmap[BITMAP_SIZE];


unsigned int kernel_start_addr = (unsigned int)&kernel_start;
unsigned int kernel_end_addr  = (unsigned int)&kernel_end;


void PrintMemoryMap(int index){
    unsigned short count = *MEMORY_MAP_COUNT;
    if(index == 0){
        int i = 0;

        WriteTerminal("Memory map entries: ");
        WriteInt(count);
        WriteTerminal("\n\n");

        while(i < count){
            WriteTerminal("Base: ");
            WriteHex((unsigned int)MEMORY_MAP_BUFFER[i].base);
            WriteTerminal("\n");

            WriteTerminal("Length: ");
            WriteHex((unsigned int)MEMORY_MAP_BUFFER[i].length);
            WriteTerminal("\n");

            WriteTerminal("Type: ");
            WriteInt((unsigned int)MEMORY_MAP_BUFFER[i].type);
            WriteTerminal("\n\n");
            i++;
        }

    }else if(index > 0){

        WriteTerminal("\nBase: ");
        WriteHex((unsigned int)MEMORY_MAP_BUFFER[index].base);
        WriteTerminal("\n");

        WriteTerminal("Length: ");
        WriteHex((unsigned int)MEMORY_MAP_BUFFER[index].length);
        WriteTerminal("\n");

        WriteTerminal("Type: ");
        WriteInt((unsigned int)MEMORY_MAP_BUFFER[index].type);
        WriteTerminal("\n\n");
    }else{
        Make_color(VGA_RED, system_bgcolor);
        WriteTerminal("Invalid Argument: ");
        Make_color(system_fgcolor, system_bgcolor);
        WriteTerminal("Argument should be <int>\n");
    }


}

int IsUsableMemory(int index){
    if((unsigned int)MEMORY_MAP_BUFFER[index].type == 1){
        return 1;
    }
    return 0;
}
unsigned int AlignUp(unsigned int address){
    if(address % 0x1000 != 0){
        address = (address/ 0x1000 + 1)*0x1000;
    }
    return address;
}
unsigned int AlignDown(unsigned int address){
    if(address % 0x1000 != 0){
        address = (address/ 0x1000)*0x1000;
    }
    return address;
}
void GetUsableBoundaries(int index, unsigned int *start, unsigned int *end){
    unsigned int raw_start = (unsigned int)MEMORY_MAP_BUFFER[index].base;   
    unsigned int raw_end = raw_start + (unsigned int)MEMORY_MAP_BUFFER[index].length;

    *start = AlignUp(raw_start);
    *end = AlignDown(raw_end);
}

unsigned int AddressToPage(unsigned int address){
    return address / 0x1000;
}

void MarkPageUsed(unsigned int page){
    unsigned int byteIndex = page / 8;
    unsigned int bitIndex = page % 8;

    page_bitmap[byteIndex] |= (1 << bitIndex);
}
void MarkPageFree(unsigned int page){
    unsigned int byteIndex = page / 8;
    unsigned int bitIndex = page % 8;

    page_bitmap[byteIndex] &= ~(1 << bitIndex);
}
int IsPageUsed(unsigned int page){
    unsigned int byteIndex = page / 8;
    unsigned int bitIndex = page % 8;

    return (page_bitmap[byteIndex] & (1 << bitIndex)) != 0  ;
}

void InitializeBitmap(void){
    for(int i = 0; i < BITMAP_SIZE; i++){
        page_bitmap[i] = 0xFF;
        reserved_bitmap[i] = 0xFF;
    }


    for(int i = 0; i < *MEMORY_MAP_COUNT; i++){
        if(IsUsableMemory(i)){
            unsigned int start;
            unsigned int end;
            GetUsableBoundaries(i, &start, &end);
            unsigned int address = start;
            while(address < end && address < MAX_MEMORY){
                unsigned int page;
                page = AddressToPage(address);
                MarkPageFree(page);
                MarkPageUnreserved(page);
                address = address + PAGE_SIZE;
            }
        }
    }

    ReserveRange(0x00000000, 0x00100000);
    ReserveRange(kernel_start_addr, kernel_end_addr);
}

void ReserveRange(unsigned int start, unsigned int end){
    start = AlignDown(start);
    end = AlignUp(end);
    unsigned int address = start;
    while(address < end && address < MAX_MEMORY){
        unsigned int page = AddressToPage(address);
        MarkPageUsed(page);
        MarkPageReserved(page);
        address = address + PAGE_SIZE;
    }


}
unsigned int PageToAddress(unsigned int page)
{
    return page * PAGE_SIZE;
}

unsigned int AllocPage(void)
{
    int page = 0;
    while(page < MAX_PAGES){
        if(!IsPageUsed(page)){
            MarkPageUsed(page);
            return PageToAddress(page);
        }
        page++;
    }
    return 0;
}

int FreePage(unsigned int addr){
    if(addr % PAGE_SIZE == 0 && addr >= 0x00100000 && addr < MAX_MEMORY && IsPageUsed(AddressToPage(addr)) && !IsPageReserved(AddressToPage(addr))){
        unsigned int page = AddressToPage(addr);
        MarkPageFree(page);
        return 1;
    }
    return 0;
}

void MarkPageReserved(unsigned int page){
    unsigned int byteIndex = page / 8;
    unsigned int bitIndex = page % 8;

    reserved_bitmap[byteIndex] |= (1 << bitIndex);
}
void MarkPageUnreserved(unsigned int page){
    unsigned int byteIndex = page / 8;
    unsigned int bitIndex = page % 8;

    reserved_bitmap[byteIndex] &= ~(1 << bitIndex);
}
int IsPageReserved(unsigned int page){
    unsigned int byteIndex = page / 8;
    unsigned int bitIndex = page % 8;

    return (reserved_bitmap[byteIndex] & (1 << bitIndex)) != 0  ;
}