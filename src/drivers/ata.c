#include "../io.h"
#include "ata.h"

#define ATA_PRIMARY_DRIVE_HEAD 0x1F6
#define ATA_PRIMARY_STATUS     0x1F7


void ATA_SelectSlave(void){
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xB0);
}
unsigned char ATA_ReadStatus(void){
    return inb(ATA_PRIMARY_STATUS);
}
int ATA_ReadSector(unsigned int lba, unsigned char *buffer){
    outb(0x1F6, 0xF0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)(lba));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    unsigned char status;
    do
    {
        status = inb(0x1F7);
    }
    while(status & 0x80);

    while(!(status & 0x08))
    {
        status = inb(0x1F7);
        if(status & 0x01)
        {
            return -1;
        }
    }
    unsigned short *buffer16 = (unsigned short *)buffer;

    for(int i = 0; i < 256; i++){
        buffer16[i] = inw(0x1F0);
    }
    return 0;
}
int ATA_WriteSector(unsigned int lba, const char *text){
    outb(0x1F6, 0xF0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)(lba));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));

    unsigned char status;
    do{
        status = inb(0x1F7);
    }while(status & 0x80);

    

}