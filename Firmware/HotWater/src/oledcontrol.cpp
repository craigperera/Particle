#include "../headers/oledcontrol.h"
#include "../headers/images.h"

oledcontrol::oledcontrol(sh1106 *display) {

    this->display = display;
    this->display->init();
}

/*
    When the Current System is active display the key information
*/
void oledcontrol::displayTemperature(double temp, int target, int min, bool isRunning, const char* until) {

    //  convert temperatures double to display
    String tmp = String::format("%.1fº", temp);
    String targ = String::format("%dº", target);
    String low = String::format("%dº", min);

    int tmpLeft = 30;
    int minLeft = 60;
    int targLeft = 100;

    if (temp == floor(temp)) {

        tmp = String::format("%.0fº", temp);
        tmpLeft = 36;
    }

    //  Clear Screen
    this->display->clear();

    //  display current temperature
    this->display->setFont(Droid_Sans_Bold_40);
    this->display->drawString(tmpLeft, 4, (const char*) tmp);

    //  show the time off, min and max temps
    this->display->setFont(ArialMT_Plain_16);
    this->display->drawString(5, 48, until);
    this->display->drawString(minLeft, 48, (const char*)low);
    this->display->drawString(targLeft, 48, (const char*)targ);

    if (isRunning) {

        this->display->drawBitmap(5, 14, Flame_Logo_width, Flame_Logo_height, Flame_Logo_bits);
    }

    this->display->display();
}

/*
    Current system is not active, so just display the current temperature and status
*/
void oledcontrol::displayCurrentTemperatureInActive(double temp) {

    //  convert temperatures double to display
    String tmp = String::format("%.1f", temp);

    int tmpLeft = 20;

    if (temp == floor(temp)) {

        tmp = String::format("%.0f", temp);
        tmpLeft = 36;
    }

    this->display->clear();
    this->display->setFont(Droid_Sans_50);
    this->display->drawString(tmpLeft, 4, (const char*) tmp);
    this->display->display();
}

void oledcontrol::showSetup() {

    this->display->clear();
    this->display->setFont(ArialMT_Plain_16);
    this->display->drawString(5,48, (const char*) "Initialising ...");
    this->display->display();
}

void oledcontrol::showReady() {

    //  Clear Screen
    this->display->clear();
    this->display->setFont(ArialMT_Plain_16);
    this->display->drawString(5,48, (const char*) "Ready ...");
    this->display->display();
}

void oledcontrol::showMessage(const char* message) {

    //  Clear Screen
    this->display->clear();
    this->display->setFont(ArialMT_Plain_16);
    this->display->drawString(5,48, message);
    this->display->display();
}
