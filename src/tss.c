#include "tss.h"


static TSS tss;

TSS *tss_get(void){
    return &tss;
}

void tss_set_kernel_stack(unsigned int stack_top)
{
    
    tss.esp0 = stack_top;
}

void tss_init(void){
    unsigned char *ptr = (unsigned char *)&tss;
    for (unsigned int i = 0; i < sizeof(TSS); i++) {
        ptr[i] = 0;
    }
    tss.ss0 = 0x10;
    tss.iomap_base = sizeof(TSS);
}