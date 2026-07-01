#include "RTC.h"
#include <time.h>

typedef unsigned int DWORD;

void RTCInitialize(void) {
}

DWORD get_fattime(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    return ((DWORD)(t->tm_year - 80) << 25)
         | ((DWORD)(t->tm_mon + 1) << 21)
         | ((DWORD)(t->tm_mday) << 16)
         | ((DWORD)(t->tm_hour) << 11)
         | ((DWORD)(t->tm_min) << 5)
         | ((DWORD)(t->tm_sec / 2));
}
