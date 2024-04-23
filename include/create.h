#pragma once
#include "filesys.h"

void mkdir(FAT32FileSystem* fs, const char* dirname);
void initDirectoryCluster(FAT32FileSystem* fs, unsigned int cluster, unsigned int parentCluster);
int createFile(FAT32FileSystem* fs, const char* filename);
// Initialize the directory cluster with '.' and '..'
void initDirectoryCluster(FAT32FileSystem* fs, unsigned int cluster, unsigned int parentCluster);