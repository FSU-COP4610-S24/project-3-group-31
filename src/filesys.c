// This part will read in the filesystem
#include "filesys.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
    FILE* file = fopen(fs->filename, "rb");
    fseek(file, offset, SEEK_SET);
    fread(buffer, fs->BPB_BytsPerSec, fs->BPB_SecPerClus, file);
    fclose(file);
}

unsigned int findDirectoryCluster(const void* buffer, const char* name) {
    const unsigned char* p = buffer;
    while (*p != 0) {
        if (p[11] == 0x10 || p[11] == 0x20) { 
            char entryName[12];
            strncpy(entryName, (const char*)p, 11);
            entryName[11] = '\0';
            if (strncmp(entryName, name, 11) == 0) {
                return *(unsigned short*)(p + 26) + (*(unsigned short*)(p + 20) << 16);
            }
        }
        p += 32; 
    }
    return 0;
}

void writeCluster(FAT32FileSystem* fs, unsigned int cluster, const void* buffer) {
    unsigned long offset = ((cluster - 2) * fs->BPB_SecPerClus + fs->BPB_RsvdSecCnt + (fs->BPB_NumFATs * fs->BPB_FATSz32)) * fs->BPB_BytsPerSec;
    fseek(fs->imageFile, offset, SEEK_SET);
    fwrite(buffer, fs->BPB_BytsPerSec, fs->BPB_SecPerClus, fs->imageFile);
    fflush(fs->imageFile); // Ensure data is written to the disk
}

// Check if a directory or file exists
int checkExists(const char* name, FAT32FileSystem* fs) {
    unsigned char buffer[512];  // Assuming sector size is 512 bytes for simplicity
    unsigned int cluster = fs->BPB_RootClus;  // Start from root cluster

    while (cluster != 0xFFFFFFFF) {  // 0xFFFFFFFF indicates end of cluster chain
        readCluster(fs, cluster, buffer);
        for (int i = 0; i < 512; i += 32) {  // Each directory entry is 32 bytes
            if (buffer[i] == 0) break;  // End of directory entries
            if (buffer[i] != 0xE5) {  // Entry is not free
                char entryName[12];
                strncpy(entryName, (const char*)(buffer + i), 11);
                entryName[11] = '\0';
                if (strncmp(entryName, name, 11) == 0) {
                    return 1;  // Found
                }
            }
        }
        cluster = readFATEntry(fs, cluster);  // Function to get the next cluster in the chain
    }
    return 0;  // Not found
}

// Allocate a directory entry for a file or directory
int allocateDirectoryEntry(const char* name, int isDir, FAT32FileSystem* fs) {
    unsigned char buffer[512];  // Assuming sector size is 512 bytes for simplicity
    unsigned int cluster = fs->BPB_RootClus;  // Start from root cluster

    while (cluster != 0xFFFFFFFF) {
        readCluster(fs, cluster, buffer);
        for (int i = 0; i < 512; i += 32) {
            if (buffer[i] == 0 || buffer[i] == 0xE5) {  // Empty or previously deleted entry
                // Fill the directory entry
                strncpy((char*)(buffer + i), name, 11);
                buffer[i + 11] = isDir ? 0x10 : 0x20;  // Directory or archive flag
                // Additional setup like first cluster, file size, etc. needed here
                writeCluster(fs, cluster, buffer);  // Assume this function writes the buffer back to the cluster
                return i;  // Return the index of the entry
            }
        }
        cluster = readFATEntry(fs, cluster);  // Get next cluster
    }
    return -1;  // No space available
}

// Update the FAT table to allocate a cluster
void updateFATTable(FAT32FileSystem* fs, unsigned int cluster) {
    // Assuming FAT is loaded into memory in fs->FAT
    unsigned int FATOffset = cluster * 4; // 4 bytes per FAT entry in FAT32
    unsigned int FATSecNum = fs->BPB_RsvdSecCnt + (FATOffset / fs->BPB_BytsPerSec);
    unsigned int FATEntOffset = FATOffset % fs->BPB_BytsPerSec;
    unsigned int endOfChain = 0x0FFFFFFF; // Mark cluster as end of chain, assuming cluster is the last one

    fseek(fs->imageFile, (FATSecNum * fs->BPB_BytsPerSec) + FATEntOffset, SEEK_SET);
    fwrite(&endOfChain, sizeof(unsigned int), 1, fs->imageFile);
    fflush(fs->imageFile); // Ensure data is written to the disk
}

// Helper function to read a cluster from the FAT table
unsigned int readFATEntry(FAT32FileSystem* fs, unsigned int cluster) {
    unsigned int FATOffset = cluster * 4; // 4 bytes per FAT entry in FAT32
    unsigned int FATSecNum = fs->BPB_RsvdSecCnt + (FATOffset / fs->BPB_BytsPerSec);
    unsigned int FATEntOffset = FATOffset % fs->BPB_BytsPerSec;

    unsigned int sectorBuffer[128];  // Assuming 512 bytes per sector / 4 bytes per FAT entry
    unsigned int nextCluster;

    // Read the sector containing the FAT entry
    fseek(fs->imageFile, (FATSecNum * fs->BPB_BytsPerSec) + (FATEntOffset / 4) * 4, SEEK_SET);
    fread(sectorBuffer, 4, 128, fs->imageFile);  // Read one sector (assuming 512 bytes)
    nextCluster = sectorBuffer[FATEntOffset / 4];

    return nextCluster & 0x0FFFFFFF; // Mask to get the lower 28 bits
}

// Function to find the next free cluster in the FAT32 file system
unsigned int getNextFreeCluster(FAT32FileSystem* fs) {
    unsigned int totalClusters = (fs->BPB_TotSec32 - (fs->BPB_RsvdSecCnt + fs->BPB_NumFATs * fs->BPB_FATSz32)) / fs->BPB_SecPerClus;
    unsigned int cluster;
    unsigned int nextCluster;

    for (cluster = 2; cluster < totalClusters; cluster++) {
        nextCluster = readFATEntry(fs, cluster);
        if (nextCluster == 0) {  // 0 indicates a free cluster
            return cluster;
        }
    }

    return 0xFFFFFFFF; // Indicate no free cluster found
}