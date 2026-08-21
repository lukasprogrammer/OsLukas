#ifndef MEMORY_H
#define MEMORY_H

struct memory_map_entry {
    unsigned long long base;
    unsigned long long length;
    unsigned int type;
    unsigned int attributes;
} __attribute__((packed));

void PrintMemoryMap(int index);

extern unsigned char kernel_start;
extern unsigned char kernel_end;


unsigned int AlignUp(unsigned int address);
unsigned int AlignDown(unsigned int address);
void GetUsableBoundaries(int index, unsigned int *start, unsigned int *end);
unsigned int AddressToPage(unsigned int address);
void InitializeBitmap(void);
int IsPageUsed(unsigned int page);
void MarkPageFree(unsigned int page);
void MarkPageUsed(unsigned int page);
void ReserveRange(unsigned int start, unsigned int end);
unsigned int AllocPage(void);

int FreePage(unsigned int address);

void MarkPageReserved(unsigned int page);
void MarkPageUnreserved(unsigned int page);
int IsPageReserved(unsigned int page);

#endif
