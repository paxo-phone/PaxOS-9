#include "libsystem_exceptions.hpp"

#include "libsystem.hpp"

libsystem::exceptions::Exception::Exception(const std::string& message)
{
    panic(message, false);
}

libsystem::exceptions::Exception::Exception(const char* message)
{
    panic(message, false);
}
