#ifndef SD_H
#define SD_H

#define MOUNT_POINT "/sdcard"

#ifdef ESP_PLATFORM

#include <stdbool.h>
#include <dirent.h> // for Dir and Files
#include <sys/stat.h>   // to check files

bool SD_init(void);
void SD_disconnect(void);

#endif

#endif /* SD_H */