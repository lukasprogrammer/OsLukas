#include "filesystem.h"

#include "../drivers/ata.h"
#include "../graphics/fbterminal.h"
static LukasFS_Superblock mounted_superblock;
int Superblock_Init(){
    LukasFS_Superblock sb;

    sb.version = 1;
    sb.total_sectors = 20480;
    sb.file_table_start = 1;
    sb.data_start = LUKASFS_DATA_START;

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
    if(FS_FindFile(filename) >= 0){
        return -4;
    }
    

    if(freeEntry < 0){
        return -1;
    }
    int freeSector = FAT_AllocateSector();
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
        FAT_FreeSector(freeSector);
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

int FS_WriteFile(const char *filename,
                 const unsigned char *data,
                 unsigned int size)
{
    if(size > 0 && data == 0){
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

    unsigned int oldSector = entry.start_sector;

    // TODO 1: Calculate required sectors
    unsigned int sectors = FS_SectorsNeeded(size);

    // TODO 2: Allocate a new FAT chain
    int start = FAT_AllocateChain(sectors);

    if(start < 0){
        return -4;
    }
  
    // TODO 3: Write data into the new chain
    if(FS_WriteChain((unsigned int)start, data, size) != 0){

        if(FAT_FreeChain((unsigned int)start) != 0){
            return -8;
        }

        return -5;
    }

    // TODO 4: Update file metadata
    entry.size = size;
    entry.start_sector = start;
    if(FileEntry_Write(index, &entry) != 0){

        if(FAT_FreeChain((unsigned int)start) != 0){
            return -8;
        }

        return -6;
    }

    // TODO 5: Free the old chain
    if(FAT_FreeChain(oldSector) != 0){
        return -7;
    }

    return 0;
}

int FS_ReadFile(const char *filename, unsigned char *buffer, unsigned int bufferSize){
    int index = FS_FindFile(filename);

    if(index < 0){
        return -1;
    }

    LukasFS_FileEntry entry;

    if(FileEntry_Read(index, &entry) != 0){
        return -2;
    }


    if(entry.size > bufferSize){
        return -4;
    }

    // if(FS_ReadChain(entry.start_sector, buffer, entry.size) != 0){
    //     return -3;
    // }

    int result = FS_ReadChain(entry.start_sector, buffer, entry.size);

    if(result != 0){
        return result - 10;
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
    if(FAT_FreeChain(entry.start_sector) != 0){
        return -4;
    }
    return 0;
}

int FS_Format(void){
    if(Superblock_Init() != 0){
        return -1;
    }
    unsigned char empty[512];
    for(int i = 0; i < 512; i++){
        empty[i] = 0;
    }
    for(int i = 1; i <= 169; i++){
        if(ATA_WriteSector(i, empty) != 0){
            return -2;
        };
    }
    return 0;
}

int FAT_ReadEntry(unsigned int index, unsigned int *value){
    if(index >= 20480 || value == 0){
        return -1;
    }

    unsigned char buffer[512];

    unsigned int sector = LUKASFS_FAT_START + index/128;
    unsigned int offset = (index % 128) * 4;

    if(ATA_ReadSector(sector, buffer) != 0){
        return -2;
    }
    *value = (unsigned int)buffer[offset]
        | ((unsigned int)buffer[offset + 1] << 8)
        | ((unsigned int)buffer[offset + 2] << 16)
        | ((unsigned int)buffer[offset + 3] << 24);

    return 0;
}
int FAT_WriteEntry(unsigned int index, unsigned int value){
    if(index >= 20480){
        return -1;
    }

    unsigned char buffer[512];

    unsigned int sector = LUKASFS_FAT_START + index/128;
    unsigned int offset = (index % 128) * 4;

    if(ATA_ReadSector(sector, buffer) != 0){
        return -2;
    }

    buffer[offset] = (unsigned char)(value & 0xFF);
    buffer[offset + 1] = (unsigned char)((value >> 8) & 0xFF);
    buffer[offset + 2] = (unsigned char)((value >> 16) & 0xFF);
    buffer[offset + 3] = (unsigned char)((value >> 24) & 0xFF);

    if(ATA_WriteSector(sector, buffer) != 0){
        return -3;
    }

    return 0;
}
int FAT_FindFreeSector(){
    unsigned int value;
    for(unsigned int sector = LUKASFS_DATA_START; sector < mounted_superblock.total_sectors; sector++){
        if(FAT_ReadEntry(sector, &value) != 0){
            return -1;
        }

        if(value == FAT_FREE){
            return sector;
        }
    }
    return -2;
}
int FAT_AllocateSector(void){
    int sector = FAT_FindFreeSector();
    if(sector < 0){
        return sector;
    }
    if(FAT_WriteEntry(sector, FAT_END) != 0){
        return -3;
    };
    return sector;
}
int FAT_FreeSector(unsigned int sector)
{
    if(sector < LUKASFS_DATA_START ||
    sector >= mounted_superblock.total_sectors){
        return -2;
    }

    if(FAT_WriteEntry(sector, FAT_FREE) != 0){
        return -1;
    };
    return 0;


}
unsigned int FS_SectorsNeeded(unsigned int size)
{
    if(size == 0){
        return 1;
    }
    return 1 +(size - 1)/512;
}

int FAT_LinkSectors(unsigned int current, unsigned int next)
{

    if((current >= mounted_superblock.total_sectors || current < LUKASFS_DATA_START) || (next >= mounted_superblock.total_sectors || next < LUKASFS_DATA_START) ){
        return -1;
    }


    if(FAT_WriteEntry(current, next) != 0){
        return -2;
    }

    return 0;
}

int FAT_AllocateChain(unsigned int count){

    if(count == 0){
        return -1;
    }

    int first = FAT_AllocateSector();

    if(first < 0){
        return -2;
    }

    unsigned int previous = (unsigned int)first;

    for(unsigned int i = 1; i < count ;i++){
        int next = FAT_AllocateSector();
        if(next < 0){
            if(FAT_FreeChain((unsigned int)first) != 0){
                return -5;
            }
            return -3;
        }
        if(FAT_LinkSectors(previous, (unsigned int)next) != 0){

            int cleanup_next = FAT_FreeSector((unsigned int)next);
            int cleanup_chain = FAT_FreeChain((unsigned int)first);

            if(cleanup_next != 0 || cleanup_chain != 0){
                return -5;
            }

            return -4;
        }

        previous = (unsigned int)next;
    }

    return first;


}
int FAT_FreeChain(unsigned int first)
{
    unsigned int current = first;

    while(1){


        if(current < LUKASFS_DATA_START ||
           current >= mounted_superblock.total_sectors){
            return -3;
        }

        unsigned int next;

        if(FAT_ReadEntry(current, &next) != 0){
            return -1;
        }

        if(next == FAT_FREE){
            return -4;
        }


        if(FAT_FreeSector(current)!=0){
            return -2;
        }


        if(next == FAT_END){
            break;
        }


        current = next;
    }

    return 0;
}

int FS_WriteDataSector(unsigned int sector,
                       const unsigned char *data,
                       unsigned int size)
{
    if(size > 512 || (size > 0 && data == 0)){
        return -1;
    }

    if(sector < LUKASFS_DATA_START ||
       sector >= mounted_superblock.total_sectors){
        return -2;
    }

    unsigned char buffer[512];

    for(unsigned int i = 0; i < 512; i++){
        buffer[i] = 0;
    }

    for(unsigned int i = 0; i < size; i++){
        buffer[i] = data[i];
    }


    if(ATA_WriteSector(sector, buffer) != 0){
        return -3;
    }

    return 0;
}
int FS_WriteChain(unsigned int first,
                  const unsigned char *data,
                  unsigned int size)
{
    unsigned int current = first;
    unsigned int written = 0;
    if(size > 0 && data == 0){
        return -4;
    }

    while(written < size){

        if(current < LUKASFS_DATA_START ||
        current >= mounted_superblock.total_sectors){
            return -3;
        }

        unsigned int remaining = size - written;
        unsigned int towrite = 0;

        if(remaining >= 512){
            towrite = 512;
        }else{
            towrite = remaining;
        }

        if(FS_WriteDataSector(current, data + written, towrite) != 0){
            return -1;
        }


        written += towrite;
        if(written < size){

            unsigned int next;

            if(FAT_ReadEntry(current, &next) != 0){
                return -2;
            }

            if(next == FAT_END || next == FAT_FREE ||
            next < LUKASFS_DATA_START ||
            next >= mounted_superblock.total_sectors){
                return -3;
            }

            current = next;
        }
    }

    return 0;
}
int FS_ReadChain(unsigned int first, char *buffer, unsigned int size){
    if(size > 0 && buffer == 0){
        return -1;
    }

    unsigned int current = first;
    unsigned int read = 0;

    while(read < size){
        if(current < LUKASFS_DATA_START || current >= mounted_superblock.total_sectors){
            return -2;
        }

        unsigned char sectorBuffer[512];

        if(ATA_ReadSector(current, sectorBuffer) != 0){
            return -3;
        }

        unsigned int remaining = size - read;
        unsigned int tocopy = remaining > 512 ? 512 : remaining;

        for(unsigned int i = 0; i < tocopy; i++){
            buffer[read + i] = sectorBuffer[i];
        }
        read += tocopy;

        if(read < size){
            unsigned int next;

            if(FAT_ReadEntry(current, &next) != 0){
                return -4;
            }

            if(next == FAT_END ||
            next == FAT_FREE ||
            next < LUKASFS_DATA_START ||
            next >= mounted_superblock.total_sectors){
                return -5;
            }

            current = next;
        }
    }
    return 0;
}