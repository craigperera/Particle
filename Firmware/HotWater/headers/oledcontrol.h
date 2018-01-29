#pragma once

#include "sh1106.h"
#include "application.h"
/*
    Oled Wiring

    GND     - -ve
    VCC     - +ve
    CLK     - A3
    MOSI    - A5
    RES     - D0
    DC      - D1
    CS      - A2
*/

// Pin definitions for SPI
#define OLED_RESET  D0   // RESET
#define OLED_DC     D1   // Data/Command
#define OLED_CS     A2   // Chip select

class oledcontrol {
    private:

        sh1106  *display;

    public:

        oledcontrol(sh1106 *display);

        /*
            Display Temperatures when in an Active state (A current event exists)
        */
        void displayTemperature(double temp, int target, int min, bool isRunning, const char* until);

        /*
            Display Temperatures when in an In-Active state (no current event exists)
        */
        void displayCurrentTemperatureInActive(double temp);

        /*
            Display the Initialising Message
        */
        void showSetup();

        /*
            Display the Ready Message
        */
        void showReady();

        /*
            Show a message in the status bar
        */
        void showMessage(const char* message);
};
