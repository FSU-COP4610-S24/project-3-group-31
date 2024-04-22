#pragma once
#include "filesys.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>


void cd(FAT32FileSystem* fs, const char* dirname) {
    unsigned char* buffer = (unsigned char*)(malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus));
    readCluster(fs, fs->currentCluster, buffer);
    unsigned int newCluster = findDirectoryCluster(buffer, dirname);

    if (newCluster == 0) {
        printf("Directory not found: %s\n", dirname);
    } else {
        fs->currentCluster = newCluster;
        printf("Changed directory to %s\n", dirname);
    }

    free(buffer);
}

void ls(FAT32FileSystem* fs) {
    unsigned char* buffer = (unsigned char*)(malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus));
    readCluster(fs, fs->currentCluster, buffer);
    const unsigned char* p = buffer;
    while (*p != 0) {
        if (p[11] == 0x10 || p[11] == 0x20) {
            char entryName[12];
            strncpy(entryName, (const char*)p, 11);
            entryName[11] = '\0';
            printf("%s\n", entryName);
        }
        p += 32; 
    }

    free(buffer);
}
