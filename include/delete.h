#pragma once
#include "filesys.h"

void rm(FAT32FileSystem* fs, const char* filename);
void rmdir(FAT32FileSystem* fs, const char* dirname);
void modifyDirectoryEntry(void* buffer, const char* targetName, char newStatus);
void freeClusterChain(FAT32FileSystem* fs, unsigned int startCluster);
unsigned int getNextCluster(FAT32FileSystem* fs, unsigned int currentCluster);
