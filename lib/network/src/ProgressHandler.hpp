#ifndef PROGRESSHANDLER_HPP
#define PROGRESSHANDLER_HPP

#include <functional>

namespace network
{
    typedef std::function<void(double)> ProgressHandler;
}

#endif // PROGRESSHANDLER_HPP