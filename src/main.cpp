#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_pm.h"

#endif

// ESP-IDF main
extern "C" void app_main()
{

}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    app_main();
}

#endif
