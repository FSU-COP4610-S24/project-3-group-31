#include "delete.h"
void rm(FAT32FileSystem* fs, const char* filename) {
    char formattedName[12];
    formatDirectoryName(formattedName, filename);

    DirectoryEntry* entry = findDirectoryEntry(fs, getCurrCluster(fs), formattedName, false);
    if (!entry) {
        printf("Error: File '%s' does not exist.\n", filename);
        return;
    }

    if (entry->DIR_Attr & ATTR_DIRECTORY) {
        printf("Error: '%s' is a directory, not a file.\n", filename);
        free(entry);
        return;
    }

    if (isFileOpened(fs, entry)) {
        printf("Error: File '%s' is currently opened.\n", filename);
        free(entry);
        return;
    }

    void* clusterBuffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    readCluster(fs, getCurrCluster(fs), clusterBuffer);
    modifyDirectoryEntry(clusterBuffer, formattedName, 0xE5);
    writeCluster(fs, getCurrCluster(fs), clusterBuffer);
    free(clusterBuffer);

    // NEED TO FIX
    //freeClusterChain(fs, getHILO(entry));
    free(entry);
    printf("File '%s' removed successfully.\n", filename);
}

void modifyDirectoryEntry(void* buffer, const char* targetName, char newStatus) {
    DirectoryEntry* entry = (DirectoryEntry*)buffer;
    while (entry->DIR_Name[0] != 0 && entry->DIR_Name[0] != 0xE5) { 
        if (strncmp(entry->DIR_Name, targetName, 11) == 0) {
            entry->DIR_Name[0] = newStatus;
            break;
        }
        entry++;
    }
}


void freeClusterChain(FAT32FileSystem* fs, unsigned int startCluster) {
    unsigned int cluster = startCluster;
    while (cluster != 0xFFFFFFFF) { 
        unsigned int nextCluster = getNextCluster(fs, cluster);
        freeCluster(fs, cluster);
        cluster = nextCluster;
    }
}

unsigned int getNextCluster(FAT32FileSystem* fs, unsigned int currentCluster) {
    unsigned int fatOffset = currentCluster * 4;
    unsigned int fatSector = fs->BPB_RsvdSecCnt + (fatOffset / fs->BPB_BytsPerSec);
    unsigned int entOffset = fatOffset % fs->BPB_BytsPerSec;

    unsigned char buffer[4];
    fseek(fs->imageFile, (fatSector * fs->BPB_BytsPerSec) + entOffset, SEEK_SET);
    fread(buffer, 1, 4, fs->imageFile);

    unsigned int nextCluster = *(unsigned int*)buffer;
    return nextCluster & 0x0FFFFFFF;
}



void rmdir(FAT32FileSystem* fs, const char* dirname) {
    char formattedName[12];
    formatDirectoryName(formattedName, dirname);

    DirectoryEntry* entry = findDirectoryEntry(fs, getCurrCluster(fs), formattedName, true);
    if (!entry) {
        printf("Error: Directory '%s' does not exist.\n", dirname);
        return;
    }

    if (!(entry->DIR_Attr & ATTR_DIRECTORY)) {
        printf("Error: '%s' is not a directory.\n", dirname);
        free(entry);
        return;
    }

    if (!isDirectoryEmpty(fs, getHILO(entry))) {
        printf("Error: Directory '%s' is not empty.\n", dirname);
        free(entry);
        return;
    }

    void* clusterBuffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    if (!clusterBuffer) {
        printf("Error: Memory allocation failed.\n");
        free(entry);
        return;
    }
    readCluster(fs, getCurrCluster(fs), clusterBuffer);
    modifyDirectoryEntry(clusterBuffer, formattedName, 0xE5);
    writeCluster(fs, getCurrCluster(fs), clusterBuffer);
    free(clusterBuffer);

    //freeClusterChain(fs, getHILO(entry));
    free(entry);
    printf("Directory '%s' removed successfully.\n", dirname);
}
