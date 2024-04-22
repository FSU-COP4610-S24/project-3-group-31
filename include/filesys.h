# pragma once
typedef struct {
    // Add necessary fields to represent the format of the FAT32 filesystem
    unsigned int BS_jmpBoot;
    char BS_OEMName[9];
    unsigned int BPB_BytsPerSec;
    unsigned int BPB_SecPerClus;
    unsigned int BPB_RsvdSecCnt;
    unsigned int BPB_NumFATs;
    unsigned int BPB_RootEntCnt;    // Always 0 on FAT32
    unsigned int BPB_TotSec16;      // Always 0 on FAT32
    unsigned int BPB_Media;
    unsigned int BPB_FATSz16;       // This field is always zero on FAT32, maybe discard it?
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
    unsigned int BPB_Reserved[3];   // Must be 0x0
    unsigned int BS_DrvNum;
    unsigned int BS_Reserved1;
    unsigned int BS_BootSig;
    unsigned int BS_VollD;
    char BS_VolLab[12];
    char BS_FilSysType[9];
    unsigned int Signature_word;
    unsigned int currentCluster;
    char* filename;
} FAT32FileSystem;

FAT32FileSystem* readFAT32FileSystem(const char* filename); 
void readBootSector(FAT32FileSystem* fs);
unsigned int getBytes(unsigned int offset, unsigned int size);
void getBytestoChar(unsigned int offset, unsigned int size, char* string);
unsigned int makeBigEndian(unsigned char *array, int bytes);
void readCluster(FAT32FileSystem* fs, unsigned int clusterNumber, void* buffer);
unsigned int findDirectoryCluster(const void* buffer, const char* name);
