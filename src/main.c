#include "filesys.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdio.h"


void testFunctionality(FAT32FileSystem*);

int main(int argc, const char* argv[]) {
    const char* filename = argv[1];
    FAT32FileSystem* fs = readFAT32FileSystem(filename);
    if (fs == NULL) {
        return 1;
    }
    testFunctionality(fs);
    // Use the fs structure to access and navigate through the FAT32 filesystem

    //free(fs);     Freeing causes a segfault??
    return 0;
}

void testFunctionality(FAT32FileSystem* fs) {
    // Ensure all the boot sector data has been obtaied
    printf("BS_jmpBoot:\t%x\n", fs->BS_jmpBoot);
    printf("BS_OEMName:\t%s\n", fs->BS_OEMName);
    printf("BPB_BytsPerSec:\t%x\n", fs->BPB_BytsPerSec);
    printf("BPB_SecPerClus:\t%x\n", fs->BPB_SecPerClus);
    printf("BPB_RsvdSecCnt:\t%x\n", fs->BPB_RsvdSecCnt);
    printf("BPB_NumFATs:\t%x\n", fs->BPB_NumFATs);
    printf("BPB_RootEntCnt:\t%x\n", fs->BPB_RootEntCnt);
    printf("BPB_TotSec16:\t%x\n", fs->BPB_TotSec16);
    printf("BPB_Media:\t%x\n", fs->BPB_Media);
    printf("BPB_FATSz16:\t%x\n", fs->BPB_FATSz16);
    printf("BPB_SecPerTrk:\t%x\n", fs->BPB_SecPerTrk);
    printf("BPB_NumHeads:\t%x\n", fs->BPB_NumHeads);
    printf("BPB_HiddSec:\t%x\n", fs->BPB_HiddSec);
    printf("BPB_TotSec32:\t%x\n", fs->BPB_TotSec32);
    printf("BPB_FATSz32:\t%x\n", fs->BPB_FATSz32);
    printf("BPB_ExtFlags:\t%x\n", fs->BPB_ExtFlags);
    printf("BPB_FSVer:\t%x\n", fs->BPB_FSVer);
    printf("BPB_RootClus:\t%x\n", fs->BPB_RootClus);
    printf("BPB_FSInfo:\t%x\n", fs->BPB_FSInfo);
    printf("BPB_BkBootSec:\t%x\n", fs->BPB_BkBootSec);
    printf("BPB_Reserved:\t%s\n", fs->BPB_Reserved);
    printf("BS_DrvNum:\t%x\n", fs->BS_DrvNum);
    printf("BS_Reserved1:\t%x\n", fs->BS_Reserved1);
    printf("BS_BootSig:\t%x\n", fs->BS_BootSig);
    printf("BS_VollD:\t%x\n", fs->BS_VollD);
    printf("BS_VolLab:\t%s\n", fs->BS_VolLab);
    printf("BS_FilSysType:\t%s\n", fs->BS_FilSysType);
    printf("Signature_word:\t%x\n", fs->Signature_word);
}