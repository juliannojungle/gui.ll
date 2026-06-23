#ifndef FILE_HELPER_H
#define FILE_HELPER_H

/* Public interface of the SD/FatFS file helpers. Definitions live in FileHelper.c. */
#include <stdbool.h>
#include "ff.h"

bool MountSdCard(void);
bool SelectActiveDrive(void);
bool OpenFile(FIL *file, const char *filename);
void CloseFile(FIL *file);
void UnMountSdCard(void);

#endif /* FILE_HELPER_H */
