#include "filesys.h"

// Function to create a directory in the FAT32 file system
void makeDir(FAT32FileSystem* fs, const char* dirname) {
    // Check if the directory already exists
    if (checkExists(dirname, fs)) {
        printf("Error: A directory or file named '%s' already exists.\n", dirname);
        return;
    }

    // Allocate a directory entry for the new directory
    int dirIndex = allocateDirectoryEntry(dirname, 1, fs);
    if (dirIndex == -1) {
        printf("Error: No space available to create directory '%s'.\n", dirname);
        return;
    }

    // Get cluster number where this directory entry was made
    unsigned int newDirCluster = getNextFreeCluster(fs);
    if (newDirCluster == 0xFFFFFFFF) {
        printf("Error: No free clusters available.\n");
        return;
    }

    // Update FAT table for the new directory cluster
    updateFATTable(fs, newDirCluster);

    // Create `.` and `..` inside this newly created directory
    unsigned char buffer[512] = { 0 };  // Assuming a sector size of 512 bytes
    createDotEntries(buffer, newDirCluster, fs->BPB_RootClus);
    writeCluster(fs, newDirCluster, buffer);
}

// Helper function to create `.` and `..` entries
void createDotEntries(unsigned char* buffer, unsigned int selfCluster, unsigned int parentCluster) {
    // Create `.` entry
    const char* dot = ".";
    strncpy((char*)buffer, dot, 1);
    buffer[11] = 0x10; // Directory attribute
    unsigned short* clusterPtr = (unsigned short*)(buffer + 26);
    *clusterPtr = (unsigned short)(selfCluster & 0xFFFF);
    *(clusterPtr + 1) = (unsigned short)((selfCluster >> 16) & 0xFFFF);

    // Create `..` entry
    const char* dotdot = "..";
    strncpy((char*)(buffer + 32), dotdot, 2);
    buffer[32 + 11] = 0x10; // Directory attribute
    clusterPtr = (unsigned short*)(buffer + 32 + 26);
    *clusterPtr = (unsigned short)(parentCluster & 0xFFFF);
    *(clusterPtr + 1) = (unsigned short)((parentCluster >> 16) & 0xFFFF);
}