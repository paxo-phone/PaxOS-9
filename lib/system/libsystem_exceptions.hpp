#ifndef LIBSYSTEM_EXCEPTIONS_HPP
#define LIBSYSTEM_EXCEPTIONS_HPP

#include <stdexcept>

/**
 * @brief Wrapper for handled exceptions.
 *
 * Please use these exception instead of standard ones.
 */
namespace libsystem::exceptions
{
    /**
     * @brief Exception base class.
     */
    class Exception
    {
      protected:
        /**
         * @brief Show a standard exception message.
         *
         * @param message The message to display.
         */
        explicit Exception(const std::string& message);

        /**
         * @brief Show a standard exception message.
         *
         * @param message The message to display.
         */
        explicit Exception(const char* message);
    };

    /**
     * @brief Wrapper for std::runtime_error.
     *
     * Standard exception, but calls an OS panic.
     */
    class RuntimeError final : Exception, public std::runtime_error
    {
      public:
        /**
         * @brief Throw a new std::runtime_error.
         *
         * @param message The message to display.
         */
        explicit RuntimeError(const std::string& message) :
            Exception(message), runtime_error(message)
        {
        }

        /**
         * @brief Throw a new std::runtime_error.
         *
         * @param message The message to display.
         */
        explicit RuntimeError(const char* message) : Exception(message), runtime_error(message) {}
    };

    /**
     * @brief Wrapper for std::out_of_range.
     *
     * Standard exception, but calls an OS panic.
     */
    class OutOfRange final : Exception, public std::out_of_range
    {
      public:
        /**
         * @brief Throw a new std::out_of_range.
         *
         * @param message The message to display.
         */
        explicit OutOfRange(const std::string& message) : Exception(message), out_of_range(message)
        {
        }

        /**
         * @brief Throw a new std::out_of_range.
         *
         * @param message The message to display.
         */
        explicit OutOfRange(const char* message) : Exception(message), out_of_range(message) {}
    };

    /**
     * @brief Wrapper for std::invalid_argument.
     *
     * Standard exception, but calls an OS panic.
     */
    class InvalidArgument final : Exception, public std::invalid_argument
    {
      public:
        /**
         * @brief Throw a new std::invalid_argument.
         *
         * @param message The message to display.
         */
        explicit InvalidArgument(const std::string& message) :
            Exception(message), invalid_argument(message)
        {
        }

        /**
         * @brief Throw a new std::invalid_argument.
         *
         * @param message The message to display.
         */
        explicit InvalidArgument(const char* message) :
            Exception(message), invalid_argument(message)
        {
        }
    };

    /**
     * @brief Wrapper for std::logic_error.
     *
     * Standard exception, but calls an OS panic.
     */
    class LogicError final : Exception, public std::logic_error
    {
      public:
        /**
         * @brief Throw a new std::logic_error.
         *
         * @param message The message to display.
         */
        explicit LogicError(const std::string& message) : Exception(message), logic_error(message)
        {
        }

        /**
         * @brief Throw a new std::logic_error.
         *
         * @param message The message to display.
         */
        explicit LogicError(const char* message) : Exception(message), logic_error(message) {}
    };
} // namespace libsystem::exceptions

#endif // LIBSYSTEM_EXCEPTIONS_HPP
