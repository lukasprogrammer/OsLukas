#ifndef GDT_H
#define GDT_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   0x1B
#define USER_DATA_SELECTOR   0x23
#define TSS_SELECTOR         0x28

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

void gdt_init(void);

static void gdt_set_gate(
    int index,
    unsigned int base,
    unsigned int limit,
    unsigned char access,
    unsigned char granularity);


#endif