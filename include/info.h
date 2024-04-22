#pragma once
#include "filesys.h"
#include <stdio.h>

void info(FAT32FileSystem* fs){
    // Ensure all the boot sector data has been obtaied
    /*
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
    */

    printf("Bytes per Sector:\t%u\n", fs->BPB_BytsPerSec);
    printf("Sectors per Cluster:\t%u\n", fs->BPB_SecPerClus);
    printf("Root Cluster:\t\t%u\n", fs->BPB_RootClus);
    printf("Total # of Clusters in Data Region:\t%u\n", fs->BPB_TotSec32);
    printf("# of Entries in One FAT:\t%u\n", fs->BPB_FATSz32);
    printf("Size of Image (in bytes):\t%u\n", 
    (fs->BPB_TotSec32)*(fs->BPB_SecPerClus)*(fs->BPB_SecPerClus));
}