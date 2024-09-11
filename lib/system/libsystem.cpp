//
// Created by Charles on 11/08/2024.
//

#include "libsystem.hpp"

#include <hardware.hpp>
#include <iostream>
#include <sstream>

#ifdef ESP_PLATFORM

#include <esp_debug_helpers.h>
#include <FT6236G.h>

#endif

#include <standby.hpp>

#include <color.hpp>
#include <graphics.hpp>

#include "base64.hpp"

std::vector<std::string> bootErrors;
libsystem::DeviceMode deviceMode = libsystem::NORMAL;
auto systemConfig = libsystem::FileConfig(storage::Path("system/config.bfc"));

class Restart final : public std::exception {
public:
    Restart() = default;
};

void libsystem::delay(uint64_t ms) {

#ifdef ESP_PLATFORM
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    SDL_Delay(ms);
#endif

}

std::string hexToString(const uint32_t hex) {
    std::stringstream stringStream;
    stringStream << std::hex << hex;
    return stringStream.str();
}

void libsystem::panic(const std::string &message, const bool restart) {
    setScreenOrientation(graphics::PORTRAIT);

#ifdef ESP_PLATFORM
    const uint16_t screenWidth = graphics::getScreenWidth();
    const uint16_t screenHeight = graphics::getScreenHeight();
#endif

    LGFX* lcd = graphics::getLCD();
#ifdef ESP_PLATFORM
    FT6236G* touchController = graphics::getTouchController();
#endif

    std::cerr << "System panicked !" << std::endl;
    std::cerr << "- OS Version: " << OS_VERSION << std::endl;
    std::cerr << "- Message: " << message << std::endl;

    // Reset configuration
    lcd->clear(graphics::packRGB565(0, 0, 0));
    lcd->setFont(&DejaVu12);
    lcd->setCursor(0, 0);
    lcd->setTextColor(graphics::packRGB565(255, 255, 255));

    // Print OS name
    lcd->setFont(&DejaVu40);
    lcd->setTextColor(graphics::packRGB565(58, 186, 153));
    lcd->printf("%s\n", OS_NAME);
    lcd->setFont(&DejaVu12);
    lcd->setTextColor(graphics::packRGB565(255, 255, 255));

    // Print OS version
    lcd->printf("\nOS Version : %s\n\n", OS_VERSION);

    // Show error message
    lcd->printf("\n\nSystem Panic:\n");
    lcd->setTextColor(graphics::packRGB565(255, 100, 100));
    lcd->printf("    %s\n\n\n", message.c_str());
    lcd->setTextColor(graphics::packRGB565(255, 255, 255));

    // Show instructions
    lcd->printf("This is not an expected behavior, please report this to the Paxo / PaxOS team.\n\n");
    lcd->printf("Contact us :\n- https://paxo.fr/\n\n");
    lcd->printf("What you should do:\n");
    lcd->printf("- Report this issue with every possible detail (what you done, installed applications...).\n");
    lcd->printf("- Check and clean the SD Card.\n");
    lcd->printf("- Re-flash this device.\n\n");


#ifdef ESP_PLATFORM

    // Backtrace

    //backtrace_saver::re_restart_debug_t backtraceData = backtrace_saver::getCurrentBacktrace();
    std::string fullBacktraceData;

    std::cerr << "Backtrace:" << std::endl;
    lcd->printf("Backtrace:\n");

    esp_backtrace_frame_t frame;
    esp_backtrace_get_start(&frame.pc, &frame.sp, &frame.next_pc);

    do {
        const std::string frameString = "0x" + hexToString(esp_cpu_process_stack_pc(frame.pc)) + ":0x" + hexToString(frame.sp);

        fullBacktraceData += frameString + ";";

        std::cerr << "- " << frameString << std::endl;
        lcd->printf("- %s\n", frameString.c_str());
    } while (esp_backtrace_get_next_frame(&frame));

#endif

    // Show hint
    lcd->printf("\n\nThe device will restart in 5 seconds.");

    // Vibrate to alert user
    for (uint8_t i = 0; i < 3; i++) {
        hardware::setVibrator(true);
        delay(100);
        hardware::setVibrator(false);
        delay(200);
    }

    constexpr uint64_t panicDelay = 5000;

#ifdef ESP_PLATFORM
    // Show QR code
    const std::string qrCodeData = "https://paxo.fr/panic/" + base64::to_base64(fullBacktraceData);

    std::cerr << "QR Code Link: " << qrCodeData << std::endl;

    const uint16_t qrCodeWidth = screenWidth / 2;

    bool isQRShown = false;

    // Send hash in QR ?
    // esp_partition_get_sha256(esp_ota_get_running_partition(), nullptr);

    // Wait 5 seconds and check touch
    for (int i = panicDelay; i > 0; i--) {
        TOUCHINFO touchInfo;
        touchController->getSamples(&touchInfo);

        if (!isQRShown && touchInfo.count > 0) {
            lcd->qrcode(qrCodeData.c_str(), screenWidth - qrCodeWidth - 30, screenHeight - qrCodeWidth - 30, qrCodeWidth);

            isQRShown = true;
        }

        delay(1);
    }
#else

    delay(panicDelay);

#endif

    if (restart) {
        libsystem::restart(true, 0, true);
    }
}

void libsystem::log(const std::string &message) {
    std::cout << "[LOG] " << message << std::endl;
}

void libsystem::registerBootError(const std::string& message) {
    bootErrors.emplace_back(message);

    log("[Boot Error] " + message);
}

bool libsystem::hasBootErrors() {
    return !bootErrors.empty();
}

void libsystem::displayBootErrors() {
    LGFX* lcd = graphics::getLCD();

    lcd->setFont(&DejaVu18);
    lcd->setTextColor(graphics::packRGB565(255, 100, 100));

    const int32_t screenWidth = graphics::getScreenWidth();
    const int32_t screenHeight = graphics::getScreenHeight();

    const int32_t fontHeight = lcd->fontHeight();

    // Draw every boot errors
    for (int32_t i = 0; i < bootErrors.size(); i++) {
        const std::string& message = bootErrors[i];

        lcd->setCursor(
            static_cast<int32_t>(0.5 * static_cast<double>(screenWidth - lcd->textWidth(message.c_str()))),
            screenHeight - static_cast<int32_t>(bootErrors.size() - i + 1) * fontHeight
        );

        lcd->print(message.c_str());
    }
}

void libsystem::restart(bool silent, const uint64_t timeout, const bool saveBacktrace) {
    if (timeout > 0) {
        delay(timeout);
    }

#ifdef ESP_PLATFORM
    if (saveBacktrace) {
        throw Restart();
    }

    esp_restart();
#endif
}

void libsystem::setDeviceMode(const DeviceMode mode) {
    deviceMode = mode;

    switch (mode) {
        case NORMAL:
            StandbyMode::restorePower();
            graphics::setBrightness(graphics::getBrightness());
            break;
        case SLEEP:
            graphics::setBrightness(0x00);
            StandbyMode::savePower();
            break;
    }
}

libsystem::DeviceMode libsystem::getDeviceMode() {
    return deviceMode;
}

libsystem::FileConfig libsystem::getSystemConfig() {
    return systemConfig;
}

libsystem::exceptions::RuntimeError::RuntimeError(const std::string &message): runtime_error(message) {
    panic(message, false);
}

libsystem::exceptions::RuntimeError::RuntimeError(const char* message): runtime_error(message) {
    panic(message, false);
}

libsystem::exceptions::OutOfRange::OutOfRange(const std::string &message): out_of_range(message) {
    panic(message, false);
}

libsystem::exceptions::OutOfRange::OutOfRange(const char *message): out_of_range(message) {
    panic(message, false);
}

libsystem::exceptions::InvalidArgument::InvalidArgument(const std::string &message): invalid_argument(message) {
    panic(message, false);
}

libsystem::exceptions::InvalidArgument::InvalidArgument(const char *message): invalid_argument(message) {
    panic(message, false);
}
