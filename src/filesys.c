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

    // use mmap to map the file to memory
    fs->currentCluster = fs->BPB_RootClus;
    return fs;
}

void readBootSector(FAT32FileSystem* fs) {
    fs->BS_jmpBoot = getBytes(0, 3);
    getBytestoChar(3, 8, fs->BS_OEMName);
    fs->BPB_BytsPerSec  = getBytes(11, 2);
    fs->BPB_SecPerClus  = getBytes(13, 1);
    fs->BPB_RsvdSecCnt  = getBytes(14, 2);
    fs->BPB_NumFATs     = getBytes(16, 1);
    fs->BPB_RootEntCnt  = getBytes(17, 2);
    fs->BPB_TotSec16    = getBytes(19, 2);
    fs->BPB_Media       = getBytes(21, 1);
    fs->BPB_FATSz16     = getBytes(22, 2);
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
    //fs->BPB_Reserved    = getBytes(52, 12);
    (fs->BPB_Reserved)[2] = (fs->BPB_Reserved)[1] = (fs->BPB_Reserved)[0] = 0x0;
    fs->BS_DrvNum       = getBytes(64, 1);
    fs->BS_Reserved1    = getBytes(65, 1);
    fs->BS_BootSig      = getBytes(66, 1);
    fs->BS_VollD        = getBytes(67, 4);
    getBytestoChar(71, 11, fs->BS_VolLab);
    getBytestoChar(82, 8, fs->BS_FilSysType);
    fs->Signature_word  = getBytes(510, 2);
    fs->currentCluster = NULL;
    fs->imageFile = imageFile;
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
    unsigned long offset = ((clusterNumber - 2) * fs->BPB_SecPerClus + fs->BPB_RsvdSecCnt + (fs->BPB_NumFATs * fs->BPB_FATSz32)) * fs->BPB_BytsPerSec;
    fseek(fs->imageFile, offset, SEEK_SET);
    fread(buffer, fs->BPB_BytsPerSec, fs->BPB_SecPerClus, fs->imageFile);
}

unsigned int findDirectoryCluster(const void* buffer, const char* name) {
    char formattedName[12];
    memset(formattedName, ' ', 11); // formatted name
    formattedName[11] = '\0';

    int nameLen = strlen(name);
    for (int i = 0; i < nameLen && i < 8; i++) {
        formattedName[i] = toupper(name[i]);
    }

    const unsigned char* p = buffer;
    while (*p != 0 && *p != 0xE5) {  //0xE5 marks a deleted file entry, 0x00 marks end of directory entries
        if ((p[11] & 0x10) && !(p[11] & 0x08)) { //check if it's a directory and not a volume label
            if (strncmp((const char*)p, formattedName, 11) == 0) {
                unsigned int high = *(unsigned short*)(p + 20);
                unsigned int low = *(unsigned short*)(p + 26);
                return (high << 16) | low;
            }
        }
        p += 32; 
    }
    return 0;
}

void writeCluster(FAT32FileSystem* fs, unsigned int clusterNumber, void* buffer) {
    if (!fs || !buffer) {
        fprintf(stderr, "Invalid filesystem or buffer.\n");
        return;
    }

    // Calculate the byte offset to the start of the desired cluster
    unsigned long clusterOffset = ((clusterNumber - 2) * fs->BPB_SecPerClus + fs->BPB_RsvdSecCnt + (fs->BPB_NumFATs * fs->BPB_FATSz32)) * fs->BPB_BytsPerSec;

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

    // Find a free entry in the directory
    int found = 0;
    for (int i = 0; i < fs->BPB_SecPerClus * fs->BPB_BytsPerSec / sizeof(DirectoryEntry); i++) {
        if (entry[i].DIR_Name[0] == 0x00 || entry[i].DIR_Name[0] == 0xE5) { // Free or deleted entry
            memset(&entry[i], 0, sizeof(DirectoryEntry));
            memcpy(entry[i].DIR_Name, entryName, strlen(entryName));
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


