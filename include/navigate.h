#pragma once
#include "filesys.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

bool cd(FAT32FileSystem* fs, const char* dirname) {
    unsigned char* buffer = (unsigned char*)(malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus));
    readCluster(fs, getCurrCluster(fs), buffer);
    DirectoryEntry* newDir = findDirectoryCluster(buffer, dirname, true);

    if (newDir == NULL) {
        printf("Directory not found: %s\n", dirname);
        free(buffer);
        return false;
    } else {
        updateCurrCluster(fs, newDir);
        printf("Changed directory to %s\n", dirname);
    }

    free(buffer);
    return true;
}

void ls(FAT32FileSystem* fs) {
    unsigned char* buffer = (unsigned char*)(malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus));
    readCluster(fs, getCurrCluster(fs), buffer);
    const unsigned char* p = buffer;
    while (*p != 0) {
        if (*p == 0xE5) {
            p += 32;
            continue;
        }
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
