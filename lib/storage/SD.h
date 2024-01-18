#ifndef SD_H
#define SD_H

#include <stdbool.h>

#define MOUNT_POINT "/sdcard"

bool SD_init(void);
void SD_disconnect(void);

#endif /* SD_H */