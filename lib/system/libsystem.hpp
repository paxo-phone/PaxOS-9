//
// Created by Charles on 11/08/2024.
//

#ifndef LIBSYSTEM_HPP
#define LIBSYSTEM_HPP

#include <cstdint>
#include <stdexcept>
#include <string>

#include <FileConfig.hpp>

// Replace this in "platformio.ini" ?
#define OS_NAME "PaxOS"

#define OS_VERSION_MAJOR "9"
#define OS_VERSION_MINOR "0"
#define OS_VERSION_PATCH "0"
#define OS_VERSION_BUILD "0"
#define OS_VERSION_CODENAME "Red Panic"

#define OS_VERSION                                                                                                     \
    OS_VERSION_MAJOR "." OS_VERSION_MINOR "." OS_VERSION_PATCH "-" OS_VERSION_BUILD " (" OS_VERSION_CODENAME ")"

/**
 * @brief System interactions.
 *
 * libsystem contains useful functions and classes to manage the system.
 */
namespace libsystem {
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
    void panic(const std::string &message, bool restart = true);

    /**
     * @brief Print a message in the console.
     *
     * @param message The log message.
     *
     * @todo Save logs.
     */
    void log(const std::string &message);

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
    void registerBootError(const std::string &message);

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
    enum DeviceMode { NORMAL, SLEEP };

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
    namespace exceptions {
        /**
         * @brief Wrapper for std::runtime_error.
         *
         * Standard exception, but calls an OS panic.
         */
        class RuntimeError final : public std::runtime_error {
        public:
            /**
             * @brief Throw a new std::runtime_error.
             *
             * @param message The message to display.
             */
            explicit RuntimeError(const std::string &message);

            /**
             * @brief Throw a new std::runtime_error.
             *
             * @param message The message to display.
             */
            explicit RuntimeError(const char *message);
        };

        /**
         * @brief Wrapper for std::out_of_range.
         *
         * Standard exception, but calls an OS panic.
         */
        class OutOfRange final : public std::out_of_range {
        public:
            /**
             * @brief Throw a new std::out_of_range.
             *
             * @param message The message to display.
             */
            explicit OutOfRange(const std::string &message);

            /**
             * @brief Throw a new std::out_of_range.
             *
             * @param message The message to display.
             */
            explicit OutOfRange(const char *message);
        };

        /**
         * @brief Wrapper for std::invalid_argument.
         *
         * Standard exception, but calls an OS panic.
         */
        class InvalidArgument final : public std::invalid_argument {
        public:
            /**
             * @brief Throw a new std::invalid_argument.
             *
             * @param message The message to display.
             */
            explicit InvalidArgument(const std::string &message);

            /**
             * @brief Throw a new std::invalid_argument.
             *
             * @param message The message to display.
             */
            explicit InvalidArgument(const char *message);
        };
    } // namespace exceptions
} // namespace libsystem

#endif // LIBSYSTEM_HPP
