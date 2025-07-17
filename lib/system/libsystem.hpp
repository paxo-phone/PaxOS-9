//
// Created by Charles on 11/08/2024.
//

#ifndef LIBSYSTEM_HPP
#define LIBSYSTEM_HPP

#include <FileConfig.hpp>
#include <color.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>

// Replace this in "platformio.ini" ?
#define OS_NAME "PaxOS"

#define OS_VERSION_MAJOR "9"
#define OS_VERSION_MINOR "0"
#define OS_VERSION_PATCH "0"
#define OS_VERSION_BUILD "0"
#define OS_VERSION_CODENAME "Red Panic"

#define OS_VERSION                                                                                 \
    OS_VERSION_MAJOR "." OS_VERSION_MINOR "." OS_VERSION_PATCH "-" OS_VERSION_BUILD                \
                     " (" OS_VERSION_CODENAME ")"

/**
 * @brief System interactions.
 *
 * libsystem contains useful functions and classes to manage the system.
 */
namespace libsystem
{
    void init();

    /**
     * @brief Delay the current thread.
     *
     * @param ms The duration of the delay in milliseconds.
     */
    void delay(uint64_t ms);

    /**
     * @brief Make the OS panic and show a screen with detailed data.
     *
     * @param message The message to show.
     * @param restart If true, the device will restart safely.
     */
    void panic(const std::string& message, bool restart = true);

    /**
     * @brief Print a message in the console.
     *
     * @param message The log message.
     *
     * @todo Save logs.
     */
    void log(const std::string& message);

    /**
     * @defgroup boot_errors Error management at boot time.
     *
     * Manage and display errors before the OS is fully started.
     *
     * @{
     */

    /**
     * @brief Register a new boot error.
     *
     * @param message The boot error message.
     */
    void registerBootError(const std::string& message);

    /**
     * @brief Display boot errors.
     *
     * Display every boot errors registered, halts the OS.
     */
    void displayBootErrors();

    /**
     * @brief Returns true if a boot error has been registered.
     *
     * @return True if a boot error has been registered.
     */
    bool hasBootErrors();

    /**
     * @}
     */

    /**
     * @brief Restart the device.
     *
     * The "saveBacktrace" parameter doesn't restart the device safely,
     * but makes it crash to force a backtrace creation.
     *
     * @param silent Unknown behavior.
     * @param timeout Delay the restart, in milliseconds.
     * @param saveBacktrace If true, a backtrace will be created.
     */
    void restart(bool silent = false, uint64_t timeout = 0, bool saveBacktrace = false);

    /**
     * @brief Different device state, unique.
     */
    enum DeviceMode
    {
        NORMAL,
        SLEEP
    };

    /**
     * @brief Set the device mode.
     *
     * Set the device mode and apply the feature of the mode.
     *
     * @param mode The new mode to apply.
     */
    void setDeviceMode(DeviceMode mode);

    /**
     * @brief Get the current device mode.
     *
     * @return The current device mode.
     */
    [[nodiscard]] DeviceMode getDeviceMode();

    /**
     * @brief Get the system FileConfig.
     *
     * Use this FileConfig to store system related preferences, like settings.
     * Please use another FileConfig to store user data.
     *
     * @return The system FileConfig.
     */
    FileConfig getSystemConfig();

    /**
     * @brief Wrapper for handled exceptions.
     *
     * Please use these exception instead of standard ones.
     */
    namespace exceptions
    {
        /**
         * @brief Wrapper for std::runtime_error.
         *
         * Standard exception, but calls an OS panic.
         */
        class RuntimeError final : public std::runtime_error
        {
          public:
            /**
             * @brief Throw a new std::runtime_error.
             *
             * @param message The message to display.
             */
            explicit RuntimeError(const std::string& message);

            /**
             * @brief Throw a new std::runtime_error.
             *
             * @param message The message to display.
             */
            explicit RuntimeError(const char* message);
        };

        /**
         * @brief Wrapper for std::out_of_range.
         *
         * Standard exception, but calls an OS panic.
         */
        class OutOfRange final : public std::out_of_range
        {
          public:
            /**
             * @brief Throw a new std::out_of_range.
             *
             * @param message The message to display.
             */
            explicit OutOfRange(const std::string& message);

            /**
             * @brief Throw a new std::out_of_range.
             *
             * @param message The message to display.
             */
            explicit OutOfRange(const char* message);
        };

        /**
         * @brief Wrapper for std::invalid_argument.
         *
         * Standard exception, but calls an OS panic.
         */
        class InvalidArgument final : public std::invalid_argument
        {
          public:
            /**
             * @brief Throw a new std::invalid_argument.
             *
             * @param message The message to display.
             */
            explicit InvalidArgument(const std::string& message);

            /**
             * @brief Throw a new std::invalid_argument.
             *
             * @param message The message to display.
             */
            explicit InvalidArgument(const char* message);
        };
    } // namespace exceptions

    /**
     * @brief inner class to managa ethe setting of the paxo
     *
     */
    namespace paxoConfig
    {
        /**
         * @brief Get the Brightness from config file
         *  if settings from config file is not found, returns 0
         * @return uint8_t
         */
        uint8_t getBrightness();

        /**
         * @brief Set the Brightness object
         * if the save settings is specified, save it to the config file
         * @param brightness
         * @param save
         */
        void setBrightness(int16_t brightness, bool save = false);

        /**
         * @brief Set the StandBy Sleep in milliseconds
         * if the save parameter is specified, save it to the config file
         * @param os_millis
         * @param save
         */
        void setStandBySleepTime(uint64_t os_millis, bool save = false);

        /**
         * @brief Get the StandBy Sleep Time from the config file
         *  if settings from config file is not found, returns 0
         *
         * @return uint64_t
         */
        uint64_t getStandBySleepTime();

        /**
         * @brief get the OS version & name
         *
         * @return std::string
         */
        std::string getOSVersion();

        /**
         * @brief Set the Wifi Credentials
         *
         * @param SSID
         * @param user
         * @param passwd
         */
        void setWifiCredentials(std::string SSID, std::string user, std::string passwd);

        /**
         * @brief Get the list of Available Wifi SSID
         *
         * @return std::vector<std::string>
         */
        std::vector<std::string> getAvailableWifiSSID();

        /**
         * @brief Get the Connected Wifi
         *
         * @return std::string
         */
        std::string getConnectedWifi();

        /**
         * @brief connect to the wifi SSI, using
         *
         * @return connection success
         */
        bool connectWifi(std::string SSID, std::string passwd);

        /**
         * @brief Get the default Background Color of widgets
         *
         * @return color_t
         */
        color_t getBackgroundColor();

        /**
         * @brief get the default Text Color of widgets
         *
         * @return color_t
         */
        color_t getTextColor();

        /**
         * @brief get the default Border Color of widgets
         *
         * @return color_t
         */
        color_t getBorderColor();

        /**
         * @brief Set the default Background Color of widgets
         *
         * @param color
         * @param save
         */
        void setBackgroundColor(color_t color, bool save = false);

        /**
         * @brief Set the default Text Color of widgets
         *
         * @param color
         * @param save
         */
        void setTextColor(color_t color, bool save = false);

        /**
         * @brief Set the default Border Color of widgets
         *
         * @param color
         * @param save
         */
        void setBorderColor(color_t color, bool save = false);
    } // namespace paxoConfig

} // namespace libsystem

#endif // LIBSYSTEM_HPP
