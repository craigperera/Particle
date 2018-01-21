#include "../headers/oledcontrol.h"
#include "../headers/images.h"

oledcontrol::oledcontrol(sh1106 *display) {

    this->display = display;
    this->display->init();
}

/*
    When the Current System is active display the key information
*/
void oledcontrol::displayTemperature(double temp, double target, bool isRunning, const char* until) {

    //  convert temperatures double to display
    String tmp = String::format("%.1f", temp);
    String targ = String::format("%.1fº", target);

    int tmpLeft = 26;
    int targLeft = 90;

    if (temp == floor(temp)) {

        tmp = String::format("%.0f", temp);
        tmpLeft = 36;
    }

    if (target == floor(target)) {

        targ = String::format("%.0fº", target);
        targLeft = 100;
    }

    //  Clear Screen
    this->display->clear();

    this->display->setFont(Droid_Sans_Bold_40);
    this->display->drawString(tmpLeft, 4, (const char*) tmp);

    //  display target temperature
    this->display->setFont(ArialMT_Plain_16);
    this->display->drawString(5, 48, until);
    this->display->drawString(targLeft, 48, (const char*)targ);

    if (isRunning) {

        this->display->drawBitmap(108, 16, Flame_Logo_width, Flame_Logo_height, Flame_Logo_bits);
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
