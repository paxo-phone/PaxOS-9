#ifndef COMPLETIONHANDLER_HPP
#define COMPLETIONHANDLER_HPP

#include <functional>

namespace network
{
    typedef std::function<void(std::string)> CompletionHandler;
}

#endif // COMPLETIONHANDLER_HPP