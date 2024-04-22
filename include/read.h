#include "filesys.h"
#include "openFileEntry.h"

void lsof(FAT32FileSystem* fs);
// To accomodate preexisting path
char* getAsciiPath(FAT32FileSystem* fs, OpenFileEntry entry, char* buffer);
void openFile(FAT32FileSystem* fs, const char* filename, const char* mode);
void closeFile(FAT32FileSystem* fs, const char* filename);
void lsof(FAT32FileSystem* fs);
void lseek(FAT32FileSystem* fs, const char* filename, unsigned int offset);
void readCommand(FAT32FileSystem* fs, const char* filename, unsigned int size);
unsigned int readFile(FAT32FileSystem* fs, unsigned int startCluster, unsigned int offset, unsigned char* buffer, unsigned int bytesToRead);
