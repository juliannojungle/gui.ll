#include "FileHelper.h"
#include "HALConfig.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>

FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br) {
    if (!fp || !fp->filePointer) {
        *br = 0;
        return FR_INVALID_OBJECT;
    }
    *br = (UINT)fread(buff, 1, btr, fp->filePointer);
    return FR_OK;
}

bool MountSdCard(void) {
    DIR *dir = opendir(SD_DIRECTORY);
    if (!dir) return false;
    closedir(dir);
    return true;
}

bool SelectActiveDrive(void) {
    return true;
}

bool OpenFile(FIL *file, const char *filename) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", SD_DIRECTORY, filename);
    FILE *fp = fopen(path, "rb");
    if (!fp) return false;
    file->filePointer = fp;
    return true;
}

void CloseFile(FIL *file) {
    if (file && file->filePointer) {
        fclose(file->filePointer);
        file->filePointer = NULL;
    }
}

void UnMountSdCard(void) {
}
