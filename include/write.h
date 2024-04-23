#pragma once
#include "filesys.h"

void writeHandler(FAT32FileSystem* fs, char* filename, char* string);
unsigned int writeFile(FAT32FileSystem* fs, OpenFileEntry* file, char* string);