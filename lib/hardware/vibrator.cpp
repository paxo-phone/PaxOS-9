#include <vibrator.hpp>


namespace hardware
{    
    namespace vibrator {
        std::vector<bool> pattern;

        void play(std::vector<bool> pattern)
        {
            vibrator::pattern = pattern;
        }
        
        void thread(void* data)
        {
            #ifdef ESP_PLATFORM
            while (true)
            {
                if(GSM::state.callState == GSM::CallState::RINGING)
                {
                    delay(200); hardware::setVibrator(true); delay(100); hardware::setVibrator(false);
                }

                if(pattern.size() > 0)
                {
                    hardware::setVibrator(pattern[0]);
                    pattern.erase(pattern.begin());
                    delay(100);

                    if(pattern.size() == 0)
                    {
                        hardware::setVibrator(false);
                    }
                }

                delay(1);
            }
            #endif
        }
    }
}