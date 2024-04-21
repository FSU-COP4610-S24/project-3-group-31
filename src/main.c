#include "filesys.c"
int main(int argc, const char* argv[]) {
    const char* filename = argv[1];
    FAT32FileSystem* fs = readFAT32FileSystem(filename);
    if (fs == NULL) {
        return 1;
    }

    // Use the fs structure to access and navigate through the FAT32 filesystem

    free(fs);
    return 0;
}
