#include "filesys.h"
#include "openFileEntry.h"

void lsof(FAT32FileSystem* fs);
// To accomodate preexisting path
char* getAsciiPath(FAT32FileSystem* fs, OpenFileEntry entry, char* buffer);
void openFile(FAT32FileSystem* fs, const char* filename, const char* mode);
void closeFile(FAT32FileSystem* fs, const char* filename);
