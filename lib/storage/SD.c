#ifdef ESP_PLATFORM

#include "SD.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/gpio.h"
#include <dirent.h>

struct SD_parameters {
    
};

bool SD_init (void) {
    const char* mount_point = MOUNT_POINT;
    bool format_if_mount_failed = false;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

#ifdef PLATFORM_ESP32DEV
    int gpio_miso = 19;
    int gpio_mosi = 23;
    int gpio_sck = 18;
    int gpio_cs = 5;
#else
    int gpio_miso = 19;
    int gpio_mosi = 23;
    int gpio_sck = 18;
    int gpio_cs = 4;
#endif

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = gpio_cs;
    slot_config.host_id = host.slot;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = gpio_mosi,
        .miso_io_num = gpio_miso,
        .sclk_io_num = gpio_sck,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    gpio_set_pull_mode(gpio_mosi, GPIO_PULLUP_ONLY);

    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, 1);

    if (ret != ESP_OK) {
        puts ("[Error]: Failed to initialize SPI bus.");
        return false;
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = format_if_mount_failed,
        .max_files = 4,  ///< Max number of open files
        .allocation_unit_size = 16 * 1024};

    sdmmc_card_t *card = NULL;

    gpio_set_pull_mode(gpio_mosi, GPIO_PULLUP_ONLY);


    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config,
                                    &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            puts("[Error]: Failed to mount filesystem. Please check the SD card");
        } else {
            puts("[Error]: Failed to mount filesystem. Please check the PCB");
        }
        return false;
    }

    puts("[Success]: Successfully mounted filesystem");

    return true;
}

void SD_disconnect(void)
{
    //esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
}

#endif