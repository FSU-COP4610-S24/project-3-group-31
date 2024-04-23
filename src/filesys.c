// This part will read in the filesystem
#include "filesys.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

FILE* imageFile;
unsigned short buffer[12];

FAT32FileSystem* readFAT32FileSystem(const char* filename) {
    imageFile = fopen(filename, "rb+");     // Read and write in binary mode
    if (imageFile == NULL) {
        printf("Error: File '%s' does not exist.\n", filename);
        return NULL;
    }

    // Read the necessary data from the file and populate the FAT32FileSystem struct
    FAT32FileSystem* fs = (FAT32FileSystem*)malloc(sizeof(FAT32FileSystem));
    readBootSector(fs);

    return fs;
}

void readBootSector(FAT32FileSystem* fs) {
    fs->BS_jmpBoot = getBytes(0, 3);
    getBytestoChar(3, 8, fs->BS_OEMName);
    fs->BPB_BytsPerSec  = getBytes(11, 2);
    fs->BPB_SecPerClus  = getBytes(13, 1);
    fs->BPB_RsvdSecCnt  = getBytes(14, 2);
    fs->BPB_NumFATs     = getBytes(16, 1);
    fs->BPB_Media       = getBytes(21, 1);
    fs->BPB_SecPerTrk   = getBytes(24, 2);
    fs->BPB_NumHeads    = getBytes(26, 2);
    fs->BPB_HiddSec     = getBytes(28, 4);
    fs->BPB_TotSec32    = getBytes(32, 4);
    fs->BPB_FATSz32     = getBytes(36, 4);
    fs->BPB_ExtFlags    = getBytes(40, 2);
    fs->BPB_FSVer       = getBytes(42, 2);
    fs->BPB_RootClus    = getBytes(44, 4);
    fs->BPB_FSInfo      = getBytes(48, 2);
    fs->BPB_BkBootSec   = getBytes(50, 2);
    fs->BS_DrvNum       = getBytes(64, 1);
    fs->BS_Reserved1    = getBytes(65, 1);
    fs->BS_BootSig      = getBytes(66, 1);
    fs->BS_VollD        = getBytes(67, 4);
    getBytestoChar(71, 11, fs->BS_VolLab);
    getBytestoChar(82, 8, fs->BS_FilSysType);
    fs->Signature_word  = getBytes(510, 2);

    // Also initializes crucial file structure information
    fs->currEntry = 
    fs->imageFile = imageFile;
    fs->files_opened = 0;

    // root directory DirectoryEntry
    fs->currEntry = establishRoot(fs);
}

unsigned int getBytes(unsigned int offset, unsigned int size)
{
    fseek(imageFile, offset, SEEK_SET);
    fread(buffer, sizeof(int), size, imageFile);
    return makeBigEndian(buffer, size);
}

void getBytestoChar(unsigned int offset, unsigned int size, char* string)
{
    fseek(imageFile, offset, SEEK_SET);
    fread(buffer, sizeof(char), size, imageFile);
    // Add nullbyte for end of line
    buffer[size] = '\0';
    strcpy(string, buffer);
}

unsigned int makeBigEndian(unsigned char *array, int bytes) {
    unsigned int out = 0;
    for(int i = 0; i < bytes; i++) {
        out +=(unsigned int)(array[i] << (i * 8)); 
    }
    return out;
}

void readCluster(FAT32FileSystem* fs, unsigned int clusterNumber, void* buffer) {
    //unsigned long offset = ((clusterNumber - 2) * fs->BPB_SecPerClus + fs->BPB_RsvdSecCnt + (fs->BPB_NumFATs * fs->BPB_FATSz32)) * fs->BPB_BytsPerSec;
    unsigned long offset = getClusterOffset(fs, clusterNumber);
    fseek(fs->imageFile, offset, SEEK_SET);
    fread(buffer, fs->BPB_BytsPerSec, fs->BPB_SecPerClus, fs->imageFile);
}

DirectoryEntry* findDirectoryCluster(const void* buffer, const char* name) {
    char formattedName[12];
    formatDirectoryName(formattedName, name);  // Make sure this uses the same formatting as in `addDirectoryEntry`

    const unsigned char* p = buffer;
    while (*p != 0 && *p != 0xE5) {  // Continue past deleted entries
        if ((p[11] & ATTR_DIRECTORY) && !(p[11] & ATTR_VOLUME_ID)) {
            if (strncmp((const char*)p, formattedName, 11) == 0) {
                return makeDirEntry((void*)p);
            }
        }
        p += 32; // Move to the next directory entry
    }
    return NULL;
}

void writeCluster(FAT32FileSystem* fs, unsigned int clusterNumber, void* buffer) {
    if (!fs || !buffer) {
        fprintf(stderr, "Invalid filesystem or buffer.\n");
        return;
    }

    // Calculate the byte offset to the start of the desired cluster
    //unsigned long clusterOffset = ((clusterNumber - 2) * fs->BPB_SecPerClus + fs->BPB_RsvdSecCnt + (fs->BPB_NumFATs * fs->BPB_FATSz32)) * fs->BPB_BytsPerSec;
    unsigned long clusterOffset = getClusterOffset(fs, clusterNumber);

    if (fs->imageFile == NULL) {
        fprintf(stderr, "Error opening filesystem image file '%s'.\n", fs->filename);
        return;
    }

    // Move the file pointer to the calculated offset
    if (fseek(fs->imageFile, clusterOffset, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to cluster position in image file.\n");
        return;
    }

    // Write the buffer to the disk image
    size_t bytesToWrite = fs->BPB_BytsPerSec * fs->BPB_SecPerClus;
    if (fwrite(buffer, 1, bytesToWrite, fs->imageFile) != bytesToWrite) {
        fprintf(stderr, "Error writing to filesystem image file.\n");
    }
    else {
        printf("Cluster %u written successfully.\n", clusterNumber);
    }
}

unsigned int findFreeCluster(FAT32FileSystem* fs)
{
    if (!fs->imageFile) {
        fprintf(stderr, "Failed to open disk image.\n");
        return 0;
    }

    // Seek to the start of the FAT
    fseek(fs->imageFile, fs->BPB_RsvdSecCnt * fs->BPB_BytsPerSec, SEEK_SET);

    unsigned int totalClusters = fs->BPB_TotSec32 / fs->BPB_SecPerClus;
    unsigned int fatEntry;
    for (unsigned int i = 2; i < totalClusters; i++) { // Cluster numbers start at 2
        fread(&fatEntry, sizeof(fatEntry), 1, fs->imageFile);
        if (fatEntry == 0) {  // 0 indicates a free cluster
            return i;
        }
    }
    return 0;  // No free cluster found
}

int addDirectoryEntry(FAT32FileSystem* fs, unsigned int directoryCluster, const char* entryName, unsigned int entryCluster, int isDirectory) {
    void* clusterBuffer = malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    if (!clusterBuffer) {
        fprintf(stderr, "Memory allocation failed for directory buffer.\n");
        return 0;
    }

    readCluster(fs, directoryCluster, clusterBuffer);
    DirectoryEntry* entry = (DirectoryEntry*)clusterBuffer;
    char formattedName[12];

    // Format the entry name into FAT32 8.3 format
    formatDirectoryName(formattedName, entryName);

    // Find a free entry in the directory
    int found = 0;
    for (int i = 0; i < fs->BPB_SecPerClus * fs->BPB_BytsPerSec / sizeof(DirectoryEntry); i++) {
        if (entry[i].DIR_Name[0] == 0x00 || entry[i].DIR_Name[0] == 0xE5) { // Free or deleted entry
            memset(&entry[i], 0, sizeof(DirectoryEntry));
            memcpy(entry[i].DIR_Name, formattedName, 11);
            entry[i].DIR_Attr = isDirectory ? ATTR_DIRECTORY : ATTR_ARCHIVE;
            entry[i].DIR_FstClusHI = (entryCluster >> 16) & 0xFFFF;
            entry[i].DIR_FstClusLO = entryCluster & 0xFFFF;
            entry[i].DIR_FileSize = 0;  // Set file size to 0 for directories

            writeCluster(fs, directoryCluster, clusterBuffer);
            found = 1;
            break;
        }
    }

    free(clusterBuffer);
    return found;
}

void freeCluster(FAT32FileSystem* fs, unsigned int clusterNumber) {
    if (!fs->imageFile) {
        fprintf(stderr, "Failed to open disk image for updating FAT.\n");
        return;
    }

    // Calculate the offset in the FAT for this cluster entry
    unsigned long offset = fs->BPB_RsvdSecCnt * fs->BPB_BytsPerSec + clusterNumber * sizeof(unsigned int);
    fseek(fs->imageFile, offset, SEEK_SET);

    unsigned int zero = 0;
    fwrite(&zero, sizeof(zero), 1, fs->imageFile);  // Free the cluster
}

// Get current directory via cluster number
unsigned int getCurrCluster(FAT32FileSystem* fs) {
    return getHILO(fs->currEntry->entry);
}

void formatDirectoryName(char* dest, const char* src) {
    memset(dest, ' ', 11);  // Fill with spaces to ensure proper formatting
    int nameLength = 0, extLength = 0;
    const char* ext = strchr(src, '.');

    if (ext != NULL) {
        nameLength = ext - src;
        extLength = strlen(ext + 1);
    }
    else {
        nameLength = strlen(src);
    }
    nameLength = nameLength > 8 ? 8 : nameLength; // Limit name length to 8
    extLength = extLength > 3 ? 3 : extLength;   // Limit extension length to 3

    // Convert name part
    for (int i = 0; i < nameLength; ++i) {
        dest[i] = toupper((unsigned char)src[i]);
    }

    // Convert extension part
    for (int i = 0; i < extLength; ++i) {
        dest[8 + i] = toupper((unsigned char)ext[i + 1]);
    }
}

bool goToParent(FAT32FileSystem* fs) {
    if (fs->currEntry->prev == NULL)
        return false;
    DirEntryList* temp = fs->currEntry;
    fs->currEntry = fs->currEntry->prev;
    fs->currEntry->next = NULL;
    // Commenting out the line below fixes SIGABRT issue, but will this not create a memory leak?
    free(temp->entry);
    free(temp);
    return true;
}

void updateCurrCluster(FAT32FileSystem* fs, DirectoryEntry* curr) {
    DirEntryList* currNode = malloc(sizeof(DirEntryList));
    currNode->entry = curr;
    currNode->next = NULL;
    currNode->prev = fs->currEntry;
    currNode->depth = fs->currEntry->depth + 1;
    fs->currEntry->next = currNode;
    fs->currEntry = currNode;
}

DirectoryEntry* makeDirEntry(void* clustStart){
    DirectoryEntry* entry = malloc(sizeof(DirectoryEntry));
    if (entry != NULL) {
        memcpy(entry, clustStart, sizeof(DirectoryEntry));
    }
    return entry;
}

unsigned int getHILO(DirectoryEntry* entry) {
    unsigned int high = entry->DIR_FstClusHI;
    unsigned int low = entry->DIR_FstClusLO;
    return (high << 16) | low;
}

DirEntryList* establishRoot(FAT32FileSystem* fs)
{
    DirEntryList* rootDir = malloc(sizeof(DirectoryEntry));
    DirectoryEntry* dirEntry = malloc(sizeof(DirectoryEntry));
    rootDir->entry = dirEntry;
    strcpy(rootDir->entry->DIR_Name, "fat32.img");   // replace with filename
    rootDir->entry->DIR_FstClusHI = (fs->BPB_RootClus >> 16) & 0xFFFF;
    rootDir->entry->DIR_FstClusLO = (fs->BPB_RootClus) & 0xFFFF;
    rootDir->prev = rootDir->next = NULL;
    rootDir->depth = 0;
    rootDir->entry->DIR_Attr = 0x10;
    return rootDir;
}

unsigned long getClusterOffset(FAT32FileSystem* fs, unsigned int startCluster) {
    // Convert offset to cluster number (assuming offset is within file size limits, already checked)
    unsigned long rootDirOffset = fs->BPB_RsvdSecCnt * fs->BPB_BytsPerSec + fs->BPB_NumFATs * fs->BPB_FATSz32 * fs->BPB_BytsPerSec;
    unsigned long clusterOffset = rootDirOffset + (((startCluster >> 16) & 0xFFFF) - 2) * fs->BPB_SecPerClus * fs->BPB_BytsPerSec
                                 + (startCluster & 0xFFFF) * fs->BPB_BytsPerSec;
    // I dont know why, but the above calculation adds a 1 in the 16^9 place that shouldn't be there, so this should fix it
    // but longs should only be 8 bytes, how can there be anything in the 16^9 place?
    clusterOffset = clusterOffset & 0xFFFFFFFF;
    return clusterOffset;
}