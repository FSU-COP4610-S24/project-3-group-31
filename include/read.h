#include "read.h"
#include "filesys.h"

void lsof(FAT32FileSystem* fs);
// To accomodate preexisting path
char* getAsciiPath(unsigned int* path, unsigned int depth, char* buffer);   