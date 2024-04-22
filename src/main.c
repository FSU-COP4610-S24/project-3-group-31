#include "filesys.h"
#include "lexer.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

void testFunctionality(FAT32FileSystem*);

int main(int argc, const char* argv[]) {
    const char* filename = argv[1];
    FAT32FileSystem* fs = readFAT32FileSystem(filename);
    if (fs == NULL) {
        return 1;
    }
    // Use the fs structure to access and navigate through the FAT32 filesystem
    lexer(fs, NULL);

    free(fs);
    return 0;
}