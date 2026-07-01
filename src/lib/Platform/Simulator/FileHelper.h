#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    FILE *filePointer;
} FIL;

typedef unsigned int UINT;
typedef unsigned int DWORD;
typedef unsigned char BYTE;

typedef enum {
    FR_OK = 0,
    FR_INVALID_OBJECT = 9
} FRESULT;

FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br);

bool MountSdCard(void);
bool SelectActiveDrive(void);
bool OpenFile(FIL *file, const char *filename);
void CloseFile(FIL *file);
void UnMountSdCard(void);

#endif /* FILE_HELPER_H */
