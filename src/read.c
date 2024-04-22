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
            fs->openFileList[i].cluster = getCurrCluster(fs);
            fs->openFileList[i].fileCluster = fileCluster;
            fs->openFileList[i].offset = 0;
            fs->openFileList[i].inUse = true;
            printf("File opened: %s at cluster %u\n", formattedName, fileCluster);
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
    printf("Index\tFile Name\tMode\t\tOffset\t\tPath\n");

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

void lseek(FAT32FileSystem* fs, const char* filename, unsigned int offset) {
    char formattedName[12];
    formatDirectoryName(formattedName, filename);  // Ensure filename is in FAT32 8.3 format

    // Search for the file in the list of open files
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->openFileList[i].inUse && strncmp(fs->openFileList[i].filename, formattedName, 11) == 0) {
            // File is open, now get the file size from its directory entry
            void* clusterBuffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
            if (!clusterBuffer) {
                printf("Error: Unable to allocate memory for reading directory cluster.\n");
                return;
            }
            readCluster(fs, fs->openFileList[i].cluster, clusterBuffer);
            DirectoryEntry* entries = (DirectoryEntry*)clusterBuffer;
            unsigned int fileSize = 0;
            bool found = false;

            // Scan the directory to find the file size
            for (int j = 0; j < fs->BPB_SecPerClus * fs->BPB_BytsPerSec / sizeof(DirectoryEntry); j++) {
                if (strncmp(entries[j].DIR_Name, formattedName, 11) == 0) {
                    fileSize = entries[j].DIR_FileSize;
                    found = true;
                    break;
                }
            }

            free(clusterBuffer);

            if (!found) {
                printf("Error: Could not find the open file '%s' in its directory cluster to check size.\n", filename);
                return;
            }

            if (offset > fileSize) {
                printf("Error: Offset %u is larger than the size of the file '%s'.\n", offset, filename);
                return;
            }

            // Update the offset for the file
            fs->openFileList[i].offset = offset;
            printf("Offset of file '%s' set to %u.\n", filename, offset);
            return;
        }
    }
    printf("Error: File '%s' is not open or does not exist.\n", filename);
}

// Read doesn't work might be due to read file not calculating right spot to check or because the spot given in the first
// place is wrong, I made it so that the file saves its own cluster but it hasn't made a huge diffenrence so im kinda lost

void readCommand(FAT32FileSystem* fs, const char* filename, unsigned int size) {
    char formattedName[12];
    formatDirectoryName(formattedName, filename);

    // Search for the file in the list of open files
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->openFileList[i].inUse && strncmp(fs->openFileList[i].filename, formattedName, 11) == 0) {
            // Check if file is opened for reading
            if (!(fs->openFileList[i].mode == 1 || fs->openFileList[i].mode == 3)) { // Mode 1: Read-only, Mode 3: Read/Write
                printf("Error: File '%s' is not open for reading.\n", filename);
                return;
            }

            // Calculate the start cluster and convert the offset to the address
            unsigned int startCluster = fs->openFileList[i].fileCluster;
            unsigned int fileOffset = fs->openFileList[i].offset;
            unsigned int bytesToRead = size;

            // Read data from the file
            unsigned char* buffer = malloc(bytesToRead);
            if (!buffer) {
                printf("Error: Unable to allocate memory for reading data.\n");
                return;
            }

            unsigned int bytesRead = readFile(fs, startCluster, fileOffset, buffer, bytesToRead);

            // Print out the data
            for (unsigned int j = 0; j < bytesRead; j++) {
                printf("%c", buffer[j]);
            }
            printf("\n");

            // Update the offset
            fs->openFileList[i].offset += bytesRead;

            free(buffer);
            return;
        }
    }

    // If no open file was found or it's not the correct file
    printf("Error: File '%s' is not open or does not exist.\n", filename);
}

unsigned int readFile(FAT32FileSystem* fs, unsigned int startCluster, unsigned int offset, unsigned char* buffer, unsigned int bytesToRead) {
    // Convert offset to cluster number (assuming offset is within file size limits, already checked)
    unsigned long clusterOffset = (startCluster - 2) * fs->BPB_SecPerClus + (offset / fs->BPB_BytsPerSec) * fs->BPB_BytsPerSec;
    fseek(fs->imageFile, clusterOffset, SEEK_SET);

    // Adjust read size if it goes beyond the file size (this requires knowing the file size which should be managed elsewhere)
    // Read the data
    unsigned int bytesRead = fread(buffer, 1, bytesToRead, fs->imageFile);
    return bytesRead;
}




