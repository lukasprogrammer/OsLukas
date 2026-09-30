#ifndef FILESYSTEM_H
#define FILESYSTEM_H

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
int FS_ReadFile(const char *filename, unsigned char *buffer);
void ListFiles();
int FS_DeleteFile(const char *filename);
#endif