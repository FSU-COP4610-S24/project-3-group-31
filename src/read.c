#include "read.h"

void openFile(FAT32FileSystem* fs, const char* filename, const char* mode) {
    // Check if file is already open
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->openFileList[i].inUse && strncmp(fs->openFileList[i].filename, filename, 11) == 0) {
            printf("Error: File '%s' is already opened.\n", filename);
            return;
        }
    }

    // Find the file in the directory
    void* clusterBuffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    readCluster(fs, getCurrCluster(fs), clusterBuffer);
    DirectoryEntry* entries = (DirectoryEntry*)clusterBuffer;

    char formattedName[12];
    formatDirectoryName(formattedName, filename);
    unsigned int fileCluster = 0;

    for (int i = 0; i < fs->BPB_SecPerClus * fs->BPB_BytsPerSec / sizeof(DirectoryEntry); i++) {
        if (strncmp(entries[i].DIR_Name, formattedName, 11) == 0) {
            fileCluster = ((unsigned int)entries[i].DIR_FstClusHI << 16) | entries[i].DIR_FstClusLO;
            break;
        }
    }
    free(clusterBuffer);

    if (fileCluster == 0) {
        printf("Error: File '%s' does not exist.\n", filename);
        return;
    }

    // Check for valid flags and set mode
    int fileMode = 0;
    if (strcmp(mode, "-r") == 0) fileMode = 1;
    else if (strcmp(mode, "-w") == 0) fileMode = 2;
    else if (strcmp(mode, "-rw") == 0 || strcmp(mode, "-wr") == 0) fileMode = 3;
    else {
        printf("Error: Invalid mode '%s'.\n", mode);
        return;
    }

    // Find an empty slot in the open file list
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fs->openFileList[i].inUse) {
            strncpy(fs->openFileList[i].filename, formattedName, 11);
            fs->openFileList[i].mode = fileMode;
            fs->openFileList[i].cluster = fileCluster;
            fs->openFileList[i].offset = 0;
            fs->openFileList[i].inUse = true;
            printf("File '%s' opened successfully in mode %s.\n", filename, mode);
            return;
        }
    }

    printf("Error: Maximum open files limit reached.\n");
}

