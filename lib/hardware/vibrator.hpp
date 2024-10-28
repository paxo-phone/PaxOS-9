#include <hardware.hpp>
#include <gsm.hpp>
#include <vector>

namespace hardware
{
    namespace vibrator {
        void play(std::vector<bool> pattern);
        
        void thread(void* data);
    }
}