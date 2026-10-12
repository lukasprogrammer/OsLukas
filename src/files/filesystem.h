#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define LUKASFS_FAT_START 10
#define LUKASFS_FAT_SECTORS 160
#define LUKASFS_DATA_START 170
#define FAT_FREE 0
#define FAT_END 0xFFFFFFFF

typedef struct{
    char magic[8];
    unsigned int version;
    unsigned int total_sectors;
    unsigned int file_table_start;
    unsigned int data_start;
}LukasFS_Superblock;

typedef struct 
{
    char filename[32];
    unsigned int size;
    unsigned int start_sector;
    unsigned char used;
    unsigned char reserved[23];

}__attribute__((packed))LukasFS_FileEntry;



int FileEntry_Read(int index, LukasFS_FileEntry *entry);
int Superblock_Init();
int Superblock_Read(LukasFS_Superblock *sb);
int IsSuperBlock_Valid(LukasFS_Superblock *sb);
int FS_Mount();
int FileEntry_Write(int index, LukasFS_FileEntry *entry);
int FileEntry_FindFree();
int DataSector_FindFree();

int FS_CreateFile(const char *filename);
int FS_FindFile(const char *filename);
int FS_WriteFile(const char *filename, const unsigned char *data, unsigned int size);
int FS_ReadFile(const char *filename, unsigned char *buffer, unsigned int bufferSize);
void ListFiles();
int FS_DeleteFile(const char *filename);
int FS_Format(void);

int FAT_ReadEntry(unsigned int index, unsigned int *value);
int FAT_WriteEntry(unsigned int index, unsigned int value);
int FAT_FindFreeSector();
int FAT_AllocateSector(void);
int FAT_FreeSector(unsigned int sector);
unsigned int FS_SectorsNeeded(unsigned int size);
int FAT_LinkSectors(unsigned int current, unsigned int next);
unsigned int FS_SectorsNeeded(unsigned int size);
int FAT_AllocateChain(unsigned int count);
int FAT_FreeChain(unsigned int first);
int FAT_LinkSectors(unsigned int current, unsigned int next);

int FS_WriteChain(unsigned int first,const unsigned char *data,unsigned int size);
int FS_WriteDataSector(unsigned int sector,
                       const unsigned char *data,
                       unsigned int size);

int FS_ReadChain(unsigned int first, char *buffer, unsigned int size);                    
#endif