#ifndef SD_HPP
#define SD_HPP 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define MOUNT_POINT "/sdcard"

bool SD_init(void);
void SD_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* SD_HPP */