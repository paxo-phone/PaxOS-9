#include <gsm2.hpp>
#include <vibrator.hpp>

namespace hardware
{
    namespace vibrator
    {
        std::vector<bool> pattern;

        void play(std::vector<bool> pattern)
        {
            vibrator::pattern = pattern;
        }

        bool isPlaying()
        {
            return pattern.size() > 0;
        }

        void thread(void* data)
        {
#ifdef ESP_PLATFORM
            while (true)
            {
                if (Gsm::getCallState() == Gsm::CallState::RINGING)
                {
                    delay(200);
                    hardware::setVibrator(true);
                    delay(100);
                    hardware::setVibrator(false);
                }

                if (pattern.size() > 0)
                {
                    hardware::setVibrator(pattern[0]);
                    delay(100);
                    pattern.erase(pattern.begin());

                    if (pattern.size() == 0)
                        hardware::setVibrator(false);
                }

                delay(1);
            }
#endif
        }
    } // namespace vibrator
} // namespace hardware
