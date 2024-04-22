// This part will read in the filesystem
#include "filesys.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

FILE* imageFile;
unsigned short buffer[12];

FAT32FileSystem* readFAT32FileSystem(const char* filename) {
    imageFile = fopen(filename, "rb");
    if (imageFile == NULL) {
        printf("Error: File '%s' does not exist.\n", filename);
        return NULL;
    }

    // Read the necessary data from the file and populate the FAT32FileSystem struct
    FAT32FileSystem* fs = (FAT32FileSystem*)malloc(sizeof(FAT32FileSystem));
    readBootSector(fs);

    // use mmap to map the file to memory


    fs->currentCluster = fs->BPB_RootClus;

    fs->filename = strdup(filename);
    if (fs->filename == NULL) {
        printf("Memory allocation failed for filename.\n");
        free(fs);
        fclose(imageFile);
        return NULL;
    }

    fclose(imageFile);

    return fs;
}

void readBootSector(FAT32FileSystem* fs) {
    fs->BS_jmpBoot = getBytes(0, 3);
    getBytestoChar(3, 8, fs->BS_OEMName);
    fs->BPB_BytsPerSec  = getBytes(11, 2);
    fs->BPB_SecPerClus  = getBytes(13, 1);
    fs->BPB_RsvdSecCnt  = getBytes(14, 2);
    fs->BPB_NumFATs     = getBytes(16, 1);
    fs->BPB_RootEntCnt  = getBytes(17, 2);
    fs->BPB_TotSec16    = getBytes(19, 2);
    fs->BPB_Media       = getBytes(21, 1);
    fs->BPB_FATSz16     = getBytes(22, 2);
    fs->BPB_SecPerTrk   = getBytes(24, 2);
    fs->BPB_NumHeads    = getBytes(26, 2);
    fs->BPB_HiddSec     = getBytes(28, 4);
    fs->BPB_TotSec32    = getBytes(32, 4);
    fs->BPB_FATSz32     = getBytes(36, 4);
    fs->BPB_ExtFlags    = getBytes(40, 2);
    fs->BPB_FSVer       = getBytes(42, 2);
    fs->BPB_RootClus    = getBytes(44, 4);
    fs->BPB_FSInfo      = getBytes(48, 2);
    fs->BPB_BkBootSec   = getBytes(50, 2);
    //fs->BPB_Reserved    = getBytes(52, 12);
    (fs->BPB_Reserved)[2] = (fs->BPB_Reserved)[1] = (fs->BPB_Reserved)[0] = 0x0;
    fs->BS_DrvNum       = getBytes(64, 1);
    fs->BS_Reserved1    = getBytes(65, 1);
    fs->BS_BootSig      = getBytes(66, 1);
    fs->BS_VollD        = getBytes(67, 4);
    getBytestoChar(71, 11, fs->BS_VolLab);
    getBytestoChar(82, 8, fs->BS_FilSysType);
    fs->Signature_word  = getBytes(510, 2);
}

unsigned int getBytes(unsigned int offset, unsigned int size)
{
    fseek(imageFile, offset, SEEK_SET);
    fread(buffer, sizeof(int), size, imageFile);
    return makeBigEndian(buffer, size);
}

void getBytestoChar(unsigned int offset, unsigned int size, char* string)
{
    fseek(imageFile, offset, SEEK_SET);
    fread(buffer, sizeof(char), size, imageFile);
    // Add nullbyte for end of line
    buffer[size] = '\0';
    strcpy(string, buffer);
}

unsigned int makeBigEndian(unsigned char *array, int bytes) {
    unsigned int out = 0;
    for(int i = 0; i < bytes; i++) {
        out +=(unsigned int)(array[i] << (i * 8)); 
    }
    return out;
}

void readCluster(FAT32FileSystem* fs, unsigned int clusterNumber, void* buffer) {
    unsigned long offset = ((clusterNumber - 2) * fs->BPB_SecPerClus + fs->BPB_RsvdSecCnt + (fs->BPB_NumFATs * fs->BPB_FATSz32)) * fs->BPB_BytsPerSec;
    fseek(fs->file, offset, SEEK_SET);
    fread(buffer, fs->BPB_BytsPerSec, fs->BPB_SecPerClus, fs->file);
}

unsigned int findDirectoryCluster(const void* buffer, const char* name) {
    char formattedName[12];
    memset(formattedName, ' ', 11); // formatted name
    formattedName[11] = '\0';

    int nameLen = strlen(name);
    for (int i = 0; i < nameLen && i < 8; i++) {
        formattedName[i] = toupper(name[i]);
    }

    const unsigned char* p = buffer;
    while (*p != 0 && *p != 0xE5) {  //0xE5 marks a deleted file entry, 0x00 marks end of directory entries
        if ((p[11] & 0x10)      //  0001 0000
            && 
            !(p[11] & 0x08))    //  0000 1000
            { //check if it's a directory and not a volume label above
            if (strncmp((const char*)p, formattedName, 11) == 0) {
                unsigned int high = *(unsigned short*)(p + 20);
                unsigned int low = *(unsigned short*)(p + 26);
                return (high << 16) | low;
            }
        }
        p += 32; 
    }
    return 0;
}