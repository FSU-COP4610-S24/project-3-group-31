// This part will read in the filesystem

// read in a fat32 file system
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    // Add necessary fields to represent the format of the FAT32 filesystem
} FAT32FileSystem;

FAT32FileSystem* readFAT32FileSystem(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: File '%s' does not exist.\n", filename);
        return NULL;
    }

    // Read the necessary data from the file and populate the FAT32FileSystem structure
    FAT32FileSystem* fs = malloc(sizeof(FAT32FileSystem));

    fclose(file);

    // Return the populated FAT32FileSystem structure
    return NULL;
}
