//
// Created by Charles on 20/01/2024.
//

#include <fstream>

uint64_t getFileSize(const char* filename) {
    auto inputStream =
        std::ifstream(filename, std::ios::ate | std::ios::binary);

    return inputStream.tellg();
}

uint8_t* getFileData(const char* filename) {
    const size_t filesize = getFileSize(filename);

    auto* data = new uint8_t[filesize];

    auto inputStream = std::ifstream(filename, std::ios::binary);

    size_t i = 0;
    while (!inputStream.eof()) {
        data[i++] = inputStream.get();
    }

    return data;
}
