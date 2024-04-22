#include "filesys.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

void mkdir(FAT32FileSystem* fs, const char* dirname) {
    // Buffer to read the current directory's content
    void* buffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    if (!buffer) {
        printf("Error allocating memory for directory buffer.\n");
        return;
    }

    // Read the current cluster (assumed to be the current directory)
    readCluster(fs, fs->currentCluster, buffer);

    // Check if the directory already exists
    unsigned int dirCluster = findDirectoryCluster(buffer, dirname);
    if (dirCluster != 0) {
        printf("Error: Directory or file named '%s' already exists.\n", dirname);
        free(buffer);
        return;
    }

    // Find a free cluster for the new directory
    unsigned int newDirCluster = findFreeCluster(fs);
    if (newDirCluster == 0) {
        printf("Error: No free cluster available.\n");
        free(buffer);
        return;
    }

    // Initialize the directory cluster with '.' and '..'
    initDirectoryCluster(fs, newDirCluster, fs->currentCluster);

    // Update the parent directory to include the new directory
    if (!addDirectoryEntry(fs, fs->currentCluster, dirname, newDirCluster, true)) {
        printf("Error: Could not add new directory entry.\n");
        freeCluster(fs, newDirCluster);
    }

    free(buffer);
}

// Utility to initialize a directory cluster with '.' and '..'
void initDirectoryCluster(FAT32FileSystem* fs, unsigned int cluster, unsigned int parentCluster) {
    // Allocate buffer to represent a cluster
    void* clusterBuffer = calloc(fs->BPB_BytsPerSec, fs->BPB_SecPerClus);
    if (!clusterBuffer) {
        printf("Failed to allocate memory for directory initialization.\n");
        return;
    }

    // Create '.' entry
    DirectoryEntry* dotEntry = (DirectoryEntry*)clusterBuffer;
    strcpy(dotEntry->DIR_Name, ".          ");
    dotEntry->DIR_Attr = ATTR_DIRECTORY;
    dotEntry->DIR_FstClusHI = (cluster >> 16) & 0xFFFF;
    dotEntry->DIR_FstClusLO = cluster & 0xFFFF;

    // Create '..' entry
    DirectoryEntry* dotDotEntry = (DirectoryEntry*)((char*)clusterBuffer + sizeof(DirectoryEntry));
    strcpy(dotDotEntry->DIR_Name, "..         ");
    dotDotEntry->DIR_Attr = ATTR_DIRECTORY;
    dotDotEntry->DIR_FstClusHI = (parentCluster >> 16) & 0xFFFF;
    dotDotEntry->DIR_FstClusLO = parentCluster & 0xFFFF;

    // Write the initialized cluster back to the disk/file
    writeCluster(fs, cluster, clusterBuffer);

    free(clusterBuffer);
}