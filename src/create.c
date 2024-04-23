#include "create.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdbool.h>

void mkdir(FAT32FileSystem* fs, const char* dirname) {
    // Buffer to read the current directory's content
    void* clusterBuffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    if (!clusterBuffer) {
        printf("Error allocating memory for directory buffer.\n");
        return;
    }

    // Read the current cluster (assumed to be the current directory)
    readCluster(fs, getCurrCluster(fs), clusterBuffer);

    // Check if the directory already exists
    unsigned int dirCluster = findDirectoryCluster(clusterBuffer, dirname);
    if (dirCluster != 0) {
        printf("Error: Directory or file named '%s' already exists.\n", dirname);
        free(clusterBuffer);
        return;
    }

    // Find a free cluster for the new directory
    unsigned int newDirCluster = findFreeCluster(fs);
    if (newDirCluster == 0) {
        printf("Error: No free cluster available.\n");
        free(clusterBuffer);
        return;
    }

    // Initialize the directory cluster with '.' and '..'
    initDirectoryCluster(fs, newDirCluster, getCurrCluster(fs));

    // Update the parent directory to include the new directory
    if (!addDirectoryEntry(fs, getCurrCluster(fs), dirname, newDirCluster, true)) {
        printf("Error: Could not add new directory entry.\n");
        freeCluster(fs, newDirCluster);
    }

    free(clusterBuffer);
}

void initDirectoryCluster(FAT32FileSystem* fs, unsigned int cluster, unsigned int parentCluster) {
    // Allocate buffer to represent a cluster
    void* clusterBuffer = calloc(1, fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    if (!clusterBuffer) {
        printf("Failed to allocate memory for directory initialization.\n");
        return;
    }

    // Set up the '.' directory entry for the current directory
    DirectoryEntry* dotEntry = (DirectoryEntry*)clusterBuffer;
    memcpy(dotEntry->DIR_Name, ".          ", 11);
    dotEntry->DIR_Attr = ATTR_DIRECTORY;
    dotEntry->DIR_FstClusHI = (cluster >> 16) & 0xFFFF;
    dotEntry->DIR_FstClusLO = cluster & 0xFFFF;
    dotEntry->DIR_FileSize = 0; // Directory size is typically 0 in FAT32

    // Set up the '..' directory entry for the parent directory
    DirectoryEntry* dotDotEntry = (DirectoryEntry*)((char*)clusterBuffer + sizeof(DirectoryEntry));
    memcpy(dotDotEntry->DIR_Name, "..         ", 11);
    dotDotEntry->DIR_Attr = ATTR_DIRECTORY;
    dotDotEntry->DIR_FstClusHI = (parentCluster >> 16) & 0xFFFF;
    dotDotEntry->DIR_FstClusLO = parentCluster & 0xFFFF;
    dotDotEntry->DIR_FileSize = 0; // Directory size is typically 0

    // Write the initialized cluster back to the disk/file
    writeCluster(fs, cluster, clusterBuffer);

    free(clusterBuffer);
}

int createFile(FAT32FileSystem* fs, const char* filename) {
    if (!fs || !filename) {
        printf("Invalid parameters.\n");
        return -1;
    }

    void* clusterBuffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    if (!clusterBuffer) {
        printf("Failed to allocate memory for directory buffer.\n");
        return -1;
    }

    // Read the current directory cluster
    readCluster(fs, getCurrCluster(fs), clusterBuffer);
    DirectoryEntry* entries = (DirectoryEntry*)clusterBuffer;

    char formattedName[12];
    formatDirectoryName(formattedName, filename);

    // Check for existing entry with the same name
    for (int i = 0; i < fs->BPB_SecPerClus * fs->BPB_BytsPerSec / sizeof(DirectoryEntry); i++) {
        if (strncmp(entries[i].DIR_Name, formattedName, 11) == 0) {
            printf("Error: File '%s' already exists.\n", filename);
            free(clusterBuffer);
            return -1;
        }
    }

    // Find a free directory entry in the current directory
    unsigned int freeCluster = 0;
    int found = 0;
    for (int i = 0; i < fs->BPB_SecPerClus * fs->BPB_BytsPerSec / sizeof(DirectoryEntry); i++) {
        if (entries[i].DIR_Name[0] == 0x00 || entries[i].DIR_Name[0] == 0xE5) { // Free or deleted entry
            freeCluster = findFreeCluster(fs);
            if (freeCluster == 0) {
                printf("Error: No free clusters available.\n");
                free(clusterBuffer);
                return -1;
            }

            // Initialize the directory entry
            memset(&entries[i], 0, sizeof(DirectoryEntry));
            memcpy(entries[i].DIR_Name, formattedName, 11);
            entries[i].DIR_Attr = ATTR_ARCHIVE; // Normal file
            entries[i].DIR_FstClusHI = (freeCluster >> 16) & 0xFFFF;
            entries[i].DIR_FstClusLO = freeCluster & 0xFFFF;
            entries[i].DIR_FileSize = 0; // New file, size 0

            writeCluster(fs, getCurrCluster(fs), clusterBuffer);
            found = 1;
            break;
        }
    }

    free(clusterBuffer);
    if (!found) {
        return -1;
    }

    return 0; // Success
}
