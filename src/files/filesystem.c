#include "filesystem.h"
#include "../drivers/ata.h"
#include "../graphics/fbterminal.h"
static LukasFS_Superblock mounted_superblock;
int Superblock_Init(){
    LukasFS_Superblock sb;

    sb.version = 1;
    sb.total_sectors = 20480;
    sb.file_table_start = 1;
    sb.data_start = 10;

    sb.magic[0] = 'L';
    sb.magic[1] = 'U';
    sb.magic[2] = 'K';
    sb.magic[3] = 'A';
    sb.magic[4] = 'S';
    sb.magic[5] = 'F';
    sb.magic[6] = 'S';
    sb.magic[7] = '\0';

    unsigned char buffer[512];
    for(int i = 0; i < 512; i++){
        buffer[i] = 0;
    }

    unsigned char *sb_bytes = (unsigned char *)&sb;
    for(unsigned int i = 0; i < sizeof(LukasFS_Superblock); i++){
        buffer[i] = sb_bytes[i];
    }

    return ATA_WriteSector(0, buffer);

}

int Superblock_Read(LukasFS_Superblock *sb){
    unsigned char buffer[512];
    unsigned char *sb_bytes = (unsigned char *)sb;
    if(ATA_ReadSector(0, buffer) != 0){
        return -1;
    }

    for(unsigned int i = 0; i < sizeof(LukasFS_Superblock); i++){
        sb_bytes[i] = buffer[i];
    }
    return 0;
}
int IsSuperBlock_Valid(LukasFS_Superblock *sb){
    
    char expected[8] = {
        'L', 'U', 'K', 'A', 'S', 'F', 'S', '\0'
    };

    for(int i = 0; i < 8; i++){
        if(expected[i] != sb->magic[i]){
            return 0;
        }
    }
    return 1;
}
int FS_Mount(){
    if(Superblock_Read(&mounted_superblock) != 0){
        return -1;
    }
    if(IsSuperBlock_Valid(&mounted_superblock)){
        return 0;
    }
    return -2;
}
int FileEntry_Read(int index, LukasFS_FileEntry *entry){
    if(index < 0 || index >= 72){
        return -1;
    }
    unsigned int sector =
    mounted_superblock.file_table_start + (index / 8);

    unsigned int slot = index % 8;

    unsigned int offset =
        slot * sizeof(LukasFS_FileEntry);

    unsigned char buffer[512];

    if(ATA_ReadSector(sector, buffer) != 0){
        return -2;
    }
    unsigned char *entry_bytes = (unsigned char *)entry;
    for(int i = 0; i < sizeof(LukasFS_FileEntry); i++){
        entry_bytes[i] = buffer[offset + i];
    }
    return 0;
}
int FileEntry_Write(int index, LukasFS_FileEntry *entry){
        if(index < 0 || index >= 72){
        return -1;
    }

    unsigned int sector =
        mounted_superblock.file_table_start + (index / 8);

    unsigned int slot = index % 8;

    unsigned int offset =
        slot * sizeof(LukasFS_FileEntry);

    unsigned char buffer[512];


    if(ATA_ReadSector(sector, buffer) != 0){
        return -2;
    }

    unsigned char *entry_bytes = (unsigned char *)entry;

    for(unsigned int i = 0; i < sizeof(LukasFS_FileEntry); i++){
        buffer[offset + i] = entry_bytes[i];
 
    }

    if(ATA_WriteSector(sector, buffer) != 0){
        return -3;
    }

    return 0;
}

int FileEntry_FindFree(){
    LukasFS_FileEntry entry;
    for(int i = 0; i < 72; i++){
        int result = FileEntry_Read(i, &entry);

        if(result != 0){
            return -1;
        }
        if(entry.used == 0){
            return i;
        }

    }
    return -2;
}

int DataSector_FindFree(){
    unsigned int candidate = mounted_superblock.data_start;
    LukasFS_FileEntry entry;

    while(candidate < mounted_superblock.total_sectors){
        int sector_used = 0;
        for(int i = 0; i < 72; i++){
            int result = FileEntry_Read(i, &entry);

            if(result != 0){
                return -1;
            }
            if(entry.used == 1){
                if(entry.start_sector == candidate){
                    sector_used = 1;
                    break;
                }
            }
        }
        if(sector_used == 0){
            return candidate;
        }else if(sector_used == 1){
            candidate++;
        }
    }
    return -2;
}

int FS_CreateFile(const char *filename){
    int freeEntry = FileEntry_FindFree();
    

    if(freeEntry < 0){
        return -1;
    }
    int freeSector = DataSector_FindFree();
    if(freeSector < 0){
        return -2;
    }
    LukasFS_FileEntry entry;
    int i = 0;

    for(int j = 0; j < 32; j++){
        entry.filename[j] = 0;
    }

    while(filename[i] != '\0' && i < 31){
        entry.filename[i] = filename[i];
        i++;
    }
    for(int j = 0; j < 23; j++){
        entry.reserved[j] = 0;
    }

    entry.filename[i] = '\0';
    entry.size = 0;
    entry.start_sector = freeSector;
    entry.used = 1;

    if(FileEntry_Write(freeEntry, &entry) != 0){
        return -3;
    }
    return 0;
}

int FS_FindFile(const char *filename)
{
    LukasFS_FileEntry entry;

    for(int i = 0; i < 72; i++)
    {
        if(FileEntry_Read(i, &entry) != 0){
            return -1;
        }

        if(entry.used == 0){
            continue;
        }
        int match = 1;
        for(int j = 0; j < 32;j++){
            if(entry.filename[j] != filename[j]){
                match = 0;
                break;
            } 
            if(entry.filename[j] == '\0'){
                break;
            }
        }
        if(match == 1){
            return i;
        }
    }

    return -2;
}

int FS_WriteFile(const char *filename, const unsigned char *data, unsigned int size){
    if(size > 512){
        return -1;
    }

    int index = FS_FindFile(filename);

    if(index < 0){
        return -2;
    }
    LukasFS_FileEntry entry;
    if(FileEntry_Read(index, &entry) != 0){
        return -3;
    }
    int sector = entry.start_sector;

    unsigned char buffer[512];

    for(int i = 0; i < 512; i++){
        buffer[i] = 0;
    }
    for(unsigned int i = 0; i < size; i++){
        buffer[i] = data[i];
    }
    entry.size = size;


    if(ATA_WriteSector(sector, buffer) != 0){
        return -4;
    }

    if(FileEntry_Write(index, &entry) != 0){
        return -5;
    }
    return 0;
}

int FS_ReadFile(const char *filename, unsigned char *buffer){
    int index = FS_FindFile(filename);

    if(index < 0){
        return -1;
    }

    LukasFS_FileEntry entry;

    if(FileEntry_Read(index, &entry) != 0){
        return -2;
    }
    int sector = entry.start_sector;

    if(ATA_ReadSector(sector, buffer) != 0){
        return -3;
    }
    return entry.size;
}
void ListFiles(){
    LukasFS_FileEntry entry;
    for(int i = 0; i < 72; i++){
        int result = FileEntry_Read(i, &entry);
        if(result != 0){
            return;
        }
        if(entry.used == 1){
            FbWriteString(entry.filename);
            FbWriteString("\n");
        }

        
    }

}
int FS_DeleteFile(const char *filename){
    int index = FS_FindFile(filename);

    if(index < 0){
        return -1;
    }
    LukasFS_FileEntry entry;
    if(FileEntry_Read(index, &entry) != 0){
        return -2;
    }
    entry.used = 0;
    if(FileEntry_Write(index, &entry) != 0){
        return -3;
    }
    return 0;
}