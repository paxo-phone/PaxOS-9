#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include "clock.hpp"

namespace hardware
{
    void init();

    void setScreenPower(bool power);
    void setVibrator(bool state);
    bool getHomeButton();

    // https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
    enum I2CResponse {
        SUCCESS = 0,
        ERROR_DATA_TOO_LONG = 1,
        ERROR_NACK_ADDRESS = 2,
        ERROR_NACK_DATA = 3,
        ERROR_OTHER = 4,
        ERROR_TIMEOUT = 5
    };

    uint8_t scanI2C(int sda = -1, int scl = -1, bool begin = true);
    I2CResponse checkI2C(uint8_t address, int sda = -1, int scl = -1, bool begin = true);

    bool isCharging();

    /**
     * Namespace managing every hardware input on the PaxoPhone.
     */
    namespace input {
        enum Button {
            HOME
        };

        enum ButtonState {
            PRESSED,
            RELEASED
        };

        struct InputFrame {
            ButtonState homeButtonState = RELEASED;
        };

        /**
         * Read the last input frame.
         * Please call this ONCE by update loop.
         */
        void update();

        /**
         * Get the button state of the provided button, raw output.
         * @param button The button to check the state.
         * @return The current button state.
         */
        ButtonState getButtonState(Button button);

        /**
         * Return if the button was pressed at this frame
         * @param button The button.
         * @return True if the button was pressed this frame.
         */
        bool getButtonDown(Button button);

        /**
         * Return if the button was released at this frame
         * @param button The button.
         * @return True if the button was released this frame.
         */
        bool getButtonUp(Button button);
    }
};

#endif