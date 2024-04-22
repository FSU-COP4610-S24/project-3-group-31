#include "read.h"
#include "filesys.h"
#include "string.h"

void lsof(FAT32FileSystem* fs) {
    if (fs->files_opened == 0) {
        printf("There are no files currently opened.\n");
        return;
    }
    printf("INDEX\tMODE\t\tOFFSET\t\tPATH\n");
    for (int i = 0; i++; i < fs->files_opened) {
        printf("%u", i);

        // print Index:
        // print file name
        // print mode
        // print offset
        // print path 
    }
}


// what an ugly function this will become -_-
char* getAsciiPath(FAT32FileSystem* fs, unsigned int* path, unsigned int depth, char* asciiPath) {
    // to grab the whole cluster
    unsigned char* buffer = (char*) malloc(fs->BPB_BytsPerSec * fs->BPB_SecPerClus);
    unsigned int clustNum = 0;

    // replace hardcoded with fs->filename
    strcpy(asciiPath, "fat32.img/");

    for (int i = 0; i < depth - 1; i++)
    {
        readCluster(fs, path[1], buffer);
        do {
            if ((buffer[11] & ATTR_DIRECTORY) && !(buffer[11] & ATTR_VOLUME_ID)){
                clustNum = ((*(*unsigned short)(buffer+20)) << 16) | 
                    *((*unsigned short)(buffer+26));
            }
            buffer += 32;
        } while (clustNum != path[depth + 1])
        buffer[9] = '\0';
        strcat(asciiPath, "/");
        strcat(asciiPath, buffer);
    }

    free(buffer);
    return asciiPath;
}