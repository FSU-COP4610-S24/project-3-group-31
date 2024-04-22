#include "read.h"
#include "filesys.h"
#include "string.h"

void lsof(FAT32FileSystem* fs) {
    if (fs->files_opened == 0) {
        printf("There are no files currently opened.\n");
        return;
    }
    printf("INDEX\tMODE\t\tOFFSET\t\tPATH\n");
    for (int i = 0; i++; i < fs->files_opened) {
        printf("%u", i);

        // print Index:
        // print file name
        // print mode
        // print offset
        // print path 
    }
}


// what an ugly function this will become -_-
char* getAsciiPath(FAT32FileSystem* fs, unsigned int* path, unsigned int depth, char* asciiPath) {
    // to grab the whole cluster
    unsigned char* buffer = (char*) malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    unsigned int clustNum = 0;

    // replace hardcoded with fs->filename
    strcpy(asciiPath, "fat32.img/");

    for (int i = 0; i < depth - 1; i++)
    {
        readCluster(fs, path[1], buffer);
        do {
            if ((buffer[11] & ATTR_DIRECTORY) && !(buffer[11] & ATTR_VOLUME_ID)){
                clustNum = ((*(*unsigned short)(buffer+20)) << 16) | 
                    *((*unsigned short)(buffer+26));
            }
            buffer += 32;
        } while (clustNum != path[depth + 1])
        buffer[9] = '\0';
        strcat(asciiPath, "/");
        strcat(asciiPath, buffer);
    }

    free(buffer);
    return asciiPath;
}

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

