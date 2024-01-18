#ifndef SD_HPP
#define SD_HPP 

#include <stdbool.h>

#define MOUNT_POINT "/sdcard"

bool SD_init(void);
void SD_disconnect(void);

#endif /* SD_HPP */