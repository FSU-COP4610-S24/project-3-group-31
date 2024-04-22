#include "filesys.h"
#include "openFileEntry.h"

void openFile(FAT32FileSystem* fs, const char* filename, const char* mode);
void closeFile(FAT32FileSystem* fs, const char* filename);
void lsof(FAT32FileSystem* fs);
void lseek(FAT32FileSystem* fs, const char* filename, unsigned int offset);