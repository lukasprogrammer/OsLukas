#include "gdt.h"
#include "tss.h"

static struct gdt_entry gdt[6];
static struct gdt_ptr gdpt;

extern void gdt_load(struct gdt_ptr *ptr);
extern void tss_load(void);

static void gdt_set_gate(
    int index,
    unsigned int base,
    unsigned int limit,
    unsigned char access,
    unsigned char granularity)
{
    gdt[index].base_low =
        base & 0xFFFF;

    gdt[index].base_middle =
        (base >> 16) & 0xFF;

    gdt[index].base_high =
        (base >> 24) & 0xFF;

    gdt[index].limit_low =
        limit & 0xFFFF;

    gdt[index].granularity =
        (limit >> 16) & 0x0F;

    gdt[index].granularity |=
        granularity & 0xF0;

    gdt[index].access = access;
}

void gdt_init(void){
    gdpt.limit = sizeof(gdt) - 1;
    gdpt.base = (unsigned int)&gdt;

    // Null
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel code
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);

    // Kernel data
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);

    // User code
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF);

    // User data
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF);

    tss_init();

    TSS *tss = tss_get();
    unsigned int tss_base = (unsigned int)tss;
    unsigned int tss_limit = sizeof(TSS) - 1;

    gdt_set_gate(5, tss_base, tss_limit, 0x89, 0x00);

    gdt_load(&gdpt);
    tss_load();

}