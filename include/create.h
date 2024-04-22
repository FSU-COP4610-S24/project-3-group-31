#include "filesys.h"

void mkdir(FAT32FileSystem* fs, const char* dirname);
void createDotEntries(unsigned char* buffer, unsigned int selfCluster, unsigned int parentCluster);