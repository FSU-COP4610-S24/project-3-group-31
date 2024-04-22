# pragma once
#include <stdint.h>
#include "read.h"
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_VOLUME_ID  0x08

typedef struct {
    // Add necessary fields to represent the format of the FAT32 filesystem
    unsigned int BS_jmpBoot;
    char BS_OEMName[9];
    unsigned int BPB_BytsPerSec;
    unsigned int BPB_SecPerClus;
    unsigned int BPB_RsvdSecCnt;
    unsigned int BPB_NumFATs;
    unsigned int BPB_Media;
    unsigned int BPB_SecPerTrk;
    unsigned int BPB_NumHeads;
    unsigned int BPB_HiddSec;
    unsigned int BPB_TotSec32;
    unsigned int BPB_FATSz32;
    unsigned int BPB_ExtFlags;
    unsigned int BPB_FSVer;
    unsigned int BPB_RootClus;
    unsigned int BPB_FSInfo;
    unsigned int BPB_BkBootSec;
    unsigned int BS_DrvNum;
    unsigned int BS_Reserved1;
    unsigned int BS_BootSig;
    unsigned int BS_VollD;
    char BS_VolLab[12];
    char BS_FilSysType[9];
    unsigned int Signature_word;
    
    unsigned int path[64];  // Max depth of 64 :D
    unsigned int depth;
    char* filename;
    FILE* imageFile;
    
    Open open_files[10];
    unsigned int files_opened;
} FAT32FileSystem;

// FAT32 Directory Entry Structure
typedef struct {
    char DIR_Name[11];        // Short name (8 characters + 3 extension, padded with spaces)
    uint8_t DIR_Attr;         // Attributes
    uint8_t DIR_NTRes;        // Reserved for use by Windows NT
    uint8_t DIR_CrtTimeTenth; // Millisecond stamp at file creation time
    uint16_t DIR_CrtTime;     // Time file was created
    uint16_t DIR_CrtDate;     // Date file was created
    uint16_t DIR_LstAccDate;  // Last access date
    uint16_t DIR_FstClusHI;   // High word of the first cluster number
    uint16_t DIR_WrtTime;     // Time of last write
    uint16_t DIR_WrtDate;     // Date of last write
    uint16_t DIR_FstClusLO;   // Low word of the first cluster number
    uint32_t DIR_FileSize;    // File size in bytes
} DirectoryEntry;

FAT32FileSystem* readFAT32FileSystem(const char* filename); 
void readBootSector(FAT32FileSystem* fs);
unsigned int getBytes(unsigned int offset, unsigned int size);
void getBytestoChar(unsigned int offset, unsigned int size, char* string);
unsigned int makeBigEndian(unsigned char *array, int bytes);
void readCluster(FAT32FileSystem* fs, unsigned int clusterNumber, void* buffer);
void writeCluster(FAT32FileSystem* fs, unsigned int clusterNumber, void* buffer);
unsigned int findDirectoryCluster(const void* buffer, const char* name);
unsigned int findFreeCluster(FAT32FileSystem* fs);
int addDirectoryEntry(FAT32FileSystem* fs, unsigned int directoryCluster, const char* entryName, unsigned int entryCluster, int isDirectory);
void freeCluster(FAT32FileSystem* fs, unsigned int clusterNumber);
unsigned int getCurrCluster(FAT32FileSystem* fs);
bool goToParent(FAT32FileSystem* fs);
void updateCurrCluster(FAT32FileSystem* fs, unsigned int newClust);
void formatDirectoryName(char* dest, const char* src);
