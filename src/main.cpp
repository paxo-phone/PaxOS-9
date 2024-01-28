#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#endif

#include "graphics.hpp"
#include "gui.hpp"
#include "path.hpp"
#include "filestream.hpp"
#include <iostream>

using namespace storage;

void storageTest()
{
    if(storage::Path("/dir1").exists())
        std::cout << "[ERROR] dir doesn't exists" << std::endl;
    if(!storage::Path("/dir1").newdir())
        std::cout << "[ERROR] creating dir" << std::endl;
    if(!storage::Path("/dir1").exists())
        std::cout << "[ERROR] dir exists" << std::endl;
    if(!storage::Path("/dir1").isdir())
        std::cout << "[ERROR] is dir for a dir" << std::endl;
    if(storage::Path("/dir1").isfile())
        std::cout << "[ERROR] is file for a dir" << std::endl;
    
    if(storage::Path("/dir1/test.txt").exists())
        std::cout << "[ERROR] file doesn't exists" << std::endl;
    if(!storage::Path("/dir1/test.txt").newfile())
        std::cout << "[ERROR] creating file" << std::endl;
    if(!storage::Path("/dir1/test.txt").isfile())
        std::cout << "[ERROR] is file for a file" << std::endl;
    if(storage::Path("/dir1/test.txt").isdir())
        std::cout << "[ERROR] is dir for a file" << std::endl;

    if(!storage::Path("/dir1/test.txt").rename(Path("/dir1/test2.txt")))
        std::cout << "[ERROR] rename file" << std::endl;
    if(!storage::Path("/dir1/test2.txt").exists())
        std::cout << "[ERROR] file2 exist" << std::endl;
    if(!storage::Path("/dir1/test2.txt").remove())
        std::cout << "[ERROR] removing a file" << std::endl;
    if(storage::Path("/dir1/test2.txt").exists())
        std::cout << "[ERROR] file2 exist" << std::endl;
    
    if(!storage::Path("/dir1").remove())
        std::cout << "[ERROR] removing dir" << std::endl;
    if(storage::Path("/dir1").exists())
        std::cout << "[ERROR] dir doesn't exists" << std::endl;

    std::cout << "[INFO] Storage Benchmark Ended" << std::endl;
}

// ESP-IDF main
extern "C" void app_main()
{
    storage::init();
    storageTest();

    storage::Path path("test.txt");
    storage::FileStream stream(path.str(), storage::Mode::READ);
    std::cout << "content: " << stream.read() << std::endl;

    graphics::init();

    while (graphics::isRunning())
    {
#ifdef ESP_PLATFORM

        vTaskDelay(pdMS_TO_TICKS(1000));

#else

        SDL_Delay(1000);

#endif
    }
}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(app_main);
}

#endif
