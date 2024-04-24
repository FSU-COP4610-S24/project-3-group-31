#include "write.h"
#include <string.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void writeHandler(FAT32FileSystem* fs, char* filename, char* string) {
    // unsigned char* buffer = (unsigned char*)(malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus));
    // DirectoryEntry* newDir = findDirectoryCluster(buffer, filename, false);
    // readCluster(fs, getCurrCluster(fs), buffer);
    char formattedName[12];
    OpenFileEntry* fileEntry = NULL;
    unsigned int bytesAdded;
    formatDirectoryName(formattedName, filename);

    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (strcmp(fs->openFileList[i].filename, formattedName) == 0) {
            fileEntry = &(fs->openFileList[i]);
            if (fileEntry->inUse)
                break;
        }
    }

    if (fileEntry == NULL || !fileEntry->inUse) {
        printf("File is not open or not inside current directory.\n");
        return;
    }
    if (fileEntry->mode == 1) {
        printf("File is open in read-only mode.\n");
        return;
    }

    bytesAdded = writeFile(fs, fileEntry, string);
    
    return;
}

unsigned int writeFile(FAT32FileSystem* fs, OpenFileEntry* file, char* string) {
    unsigned long fileOffset = 0;
    unsigned int bytes;
    unsigned char* buffer = (unsigned char*)(malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus));
    
    fileOffset = getClusterOffset(fs, file->fileCluster);

    if (fseek(fs->imageFile, fileOffset, SEEK_SET) != 0) {
        printf("Error seeking to cluster position in image file.\n");
        return 0;
    }
    
    bytes = strlen(string);
    if (fwrite(string, 1, bytes, fs->imageFile) != bytes) {
        printf("Error writing to filesystem image file.\n");
        return 0;
    }

    // Update the filesize in memory
    file->fileSize += bytes;

    
    // It is essential to modify the filesize field of the file's metadata, or else it will be
    // unable to open properly after closing. This will be done eventually.
    unsigned int parentHILO = getHILO(fs->currEntry->entry);
    unsigned int offset_from_parent_cluster = 0;
    unsigned int *newSize = malloc(sizeof(unsigned int));

    readCluster(fs, parentHILO, buffer);
    DirectoryEntry* newDir = (DirectoryEntry*)buffer;
    
    while (newDir->DIR_FstClusLO != (file->fileCluster & 0xFFFF)) {
        offset_from_parent_cluster += 32;
        newDir++;
    }

    newDir->DIR_FileSize += bytes;
    *newSize = bytes;
    fseek(fs->imageFile, parentHILO + offset_from_parent_cluster + 28, SEEK_SET);
    if (fwrite(newSize, 4, 1, fs->imageFile) != 1) {
        printf("Error writing new file size to filesystem image file.\n");
        return 0;
    }

    free(newSize);
    return bytes;
}