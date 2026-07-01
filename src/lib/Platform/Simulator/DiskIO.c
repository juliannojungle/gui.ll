/*-----------------------------------------------------------------------/
/  Low level disk I/O module for Simulator (POSIX file-backed)           /
/-----------------------------------------------------------------------*/

#include "ff.h"
#include "diskio.h"
#include "HALConfig.h"

#include <stdio.h>
#include <stdbool.h>

static FILE *diskFile = NULL;

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != 0) return STA_NOINIT;

    if (diskFile != NULL) {
        fclose(diskFile);
        diskFile = NULL;
    }

    diskFile = fopen(SD_DISK_IMAGE, "rb+");
    if (diskFile == NULL) return STA_NOINIT;

    return 0;
}

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv != 0) return STA_NOINIT;
    return (diskFile != NULL) ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != 0) return RES_PARERR;
    if (diskFile == NULL) return RES_NOTRDY;

    if (fseek(diskFile, (long)(sector * 512), SEEK_SET) != 0) return RES_ERROR;

    size_t bytesRead = fread(buff, 1, count * 512, diskFile);
    if (bytesRead != count * 512) return RES_ERROR;

    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != 0) return RES_PARERR;
    if (diskFile == NULL) return RES_NOTRDY;

    if (fseek(diskFile, (long)(sector * 512), SEEK_SET) != 0) return RES_ERROR;

    size_t bytesWritten = fwrite(buff, 1, count * 512, diskFile);
    if (bytesWritten != count * 512) return RES_ERROR;

    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    if (pdrv != 0) return RES_PARERR;
    if (diskFile == NULL) return RES_NOTRDY;

    switch (cmd) {
    case CTRL_SYNC:
        fflush(diskFile);
        return RES_OK;
    case GET_SECTOR_COUNT:
        fseek(diskFile, 0, SEEK_END);
        *(LBA_t *)buff = (LBA_t)(ftell(diskFile) / 512);
        return RES_OK;
    case GET_SECTOR_SIZE:
        *(WORD *)buff = 512;
        return RES_OK;
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = 1;
        return RES_OK;
    default:
        return RES_PARERR;
    }
}

bool SDCardInit(void) {
    DSTATUS status = disk_initialize(0);
    return (status == 0);
}
