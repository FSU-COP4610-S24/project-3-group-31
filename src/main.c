#include "filesys.h"
#include "lexer.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void cleanFileSystem(FAT32FileSystem*);

int main(int argc, const char* argv[]) {
    const char* filename = argv[1];
    FAT32FileSystem* fs = readFAT32FileSystem(filename);
    if (fs == NULL) {
        return 1;
    }
    // Use the fs structure to access and navigate through the FAT32 filesystem
    lexer(fs);

    //free(fs->filename);   // Never initialized (yet)
    free(fs->imageFile);
    free(fs);
    return 0;
}

void cleanFileSystem(FAT32FileSystem* fs)
{
    int ctr = fs->currEntry->depth;
    for (int i = 1; i < fs->currEntry->depth; i++)
    {
        goToParent(fs);
    }
    free(fs->currEntry->entry);
    free(fs->currEntry);
    free(fs->imageFile);
    free(fs);
}