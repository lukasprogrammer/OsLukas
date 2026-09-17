#ifndef ATA_H
#define ATA_H

unsigned char ATA_ReadStatus(void);
void ATA_SelectSlave(void);

int ATA_ReadSector(unsigned int lba, unsigned char *buffer);
int ATA_WriteSector(unsigned int lba, const char* text);
#endif