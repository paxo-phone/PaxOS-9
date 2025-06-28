#include <hardware.hpp>
#include <vector>

namespace hardware
{
    namespace vibrator
    {
        void play(std::vector<bool> pattern);
        bool isPlaying();
        void thread(void* data);
    } // namespace vibrator
} // namespace hardware
