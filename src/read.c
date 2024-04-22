#include "read.h"
#include "string.h"

// what an ugly function this will become -_-
// FIXME: DELETE THIS FUNCTION
// Instead, change the fs struct to include a doubly linked list of the DirectoryEntry stuff
// such that the name and HI and LO numbers are retained.
// Memory technical debt.
char* getAsciiPath(FAT32FileSystem* fs, OpenFileEntry entry, char* asciiPath) {
    // to grab the whole cluster
    unsigned char* buffer = (char*) malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    unsigned int clustNum = 0;
    unsigned int depth = entry.depth;
    unsigned int* path = entry.path;
    unsigned int high, low;

    // replace hardcoded with fs->filename
    strcpy(asciiPath, "fat32.img/");
    if (depth == 0)
        return asciiPath;

    for (int i = 0; i < depth; i++)
    {
        readCluster(fs, path[i], buffer);
        do {
            if ((buffer[11] & ATTR_DIRECTORY) && !(buffer[11] & ATTR_VOLUME_ID)){
                high = *(unsigned short*)(buffer + 20);
                low = *(unsigned short*)(buffer + 26);

                clustNum = (high << 16) | low;
                    // FIXME: Error in calculating cluster number somehow
            }
            buffer += 32;
        } while (clustNum != path[i]);
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
            fs->openFileList[i].cluster = getCurrCluster(fs);
            fs->openFileList[i].offset = 0;
            fs->openFileList[i].inUse = true;
            memcpy(fs->openFileList[i].path, fs->path, fs->depth);
            fs->openFileList[i].depth = fs->depth;
            printf("File opened: %s at cluster %u\n", formattedName, getCurrCluster(fs));
            printf("File '%s' opened successfully in mode %s.\n", filename, mode);
            return;
        }
    }

    printf("Error: Maximum open files limit reached.\n");
}

void closeFile(FAT32FileSystem* fs, const char* filename) {
    char formattedName[12];
    formatDirectoryName(formattedName, filename);  // Format name to FAT32 8.3 format
    printf("Current directory cluster: %u\n", getCurrCluster(fs));


    // Attempt to find and close the file in the open files list
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        printf("Trying to close file in cluster: %u, file cluster: %u\n", getCurrCluster(fs), fs->openFileList[i].cluster);
        if (fs->openFileList[i].inUse && strncmp(fs->openFileList[i].filename, formattedName, 11) == 0) {
            // Check if the file's starting cluster matches the current directory's cluster
            // This ensures the file is in the current working directory
            if (fs->openFileList[i].cluster == getCurrCluster(fs)) {
                fs->openFileList[i].inUse = false;  // Mark the file as closed
                printf("File '%s' closed successfully.\n", filename);
                return;
            }
        }
    }

    printf("Error: File '%s' is not open or does not exist in the current directory.\n", filename);
}

void lsof(FAT32FileSystem* fs) {
    bool anyOpen = false;
    printf("Index\tFile Name\t\tMode\t\tOffset\t\tPath\n");
    char path[256];

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->openFileList[i].inUse) {
            anyOpen = true;
            char modeDesc[20];
            switch (fs->openFileList[i].mode) {
            case 1: strcpy(modeDesc, "Read-Only"); break;
            case 2: strcpy(modeDesc, "Write-Only"); break;
            case 3: strcpy(modeDesc, "Read/Write"); break;
            default: strcpy(modeDesc, "Unknown Mode"); break;
            }
            // SUPER IMPORTANT BELOW PLZ FIX

            char path[256] = "/current/path/"; // This needs to be gotten from a func we haven't made don't ship like this plox plz
            getAsciiPath(fs, fs->openFileList[i], path);

            // Fix the spacing between them also
            printf("%d\t%s\t\t%s\t%d\t\t%s\n",
                i,                          // Index
                fs->openFileList[i].filename, // File Name
                modeDesc,                   // Mode
                fs->openFileList[i].offset, // Offset
                path);                      // Path (Static example, replace with dynamic path if available)
        }
    }

    if (!anyOpen) {
        printf("No files are currently opened.\n");
    }
}


