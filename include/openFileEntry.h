#pragma once
#include <stdbool.h>
#define MAX_OPEN_FILES 10

typedef struct {
    char filename[12];       // Stored in 8.3 format
    int mode;                // Mode: 1 for read, 2 for write, 3 for read and write
    unsigned int cluster;    // Start cluster of the dir
    unsigned int offset;     // Current offset in the file
    unsigned int fileCluster; // start cluster of file
    bool inUse;              // Indicates if the entry is in use
} OpenFileEntry;