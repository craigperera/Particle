#include "../headers/code.h"
#include "../headers/definitions.h"
#include "application.h"

#include <OneWire.h>

#include <Wire.h>
#include <SPI.h>
#include "../headers/sh1106.h"
#include "../headers/images.h"
#include "../headers/fonts.h"
#include "../headers/oledcontrol.h"
#include "../headers/dallastemperature.h"

#include <GoogleHome.h>
#include <GoogleHomeDefs.h>

//  internal methods
void setupDefaultClock();
void getTemperature();
void checkEvents();
void startButtonPushed();
void setGoogleData();

//  particle functions
int receiveMessage(String message);
void publishStateListener(const char *event, const char *data);

DeviceState handleSingleCommand(GoogleCommand command, DeviceState prevState);

//  DS182B20 definitions
#define ONE_WIRE_BUS D2
#define BUTTON_PIN D3

OneWire oneWire(ONE_WIRE_BUS);
dallastemperature dallas(&oneWire);

OneWire ds = OneWire(ONE_WIRE_BUS); //  4.7K resistor required on ONE_WIRE_BUS

//  constants
const int indexer = 0;

//  variables
String version = "HWC_0.0.4c";
String localIp = "";
String lastStatus = "";

long lastSent = 0;

OptionsModel options;
GoogleHome gHome(DeviceType::Thermostat);

//  temperatures
double currentTemp = 0;
double prevTemp = 0;

bool isRunning = false;
bool nightTimeMode = false;

volatile bool overrideOn = false;

//  Display
sh1106 pDisplay(true, OLED_RESET, OLED_DC, OLED_CS);
oledcontrol oled = oledcontrol(&pDisplay);

/*
    Initialise the particle on startup
*/
void initialise() {

    Serial.begin(9600);

    //  register any variables
    Particle.function("pMessage", receiveMessage);

    //  register listener
    Particle.subscribe("publishData", publishStateListener);

    pinMode(D2, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    //  attach interrupts
    attachInterrupt(BUTTON_PIN, startButtonPushed, RISING, 5);

    //  register device Traits
    gHome.registerDeviceTrait(DeviceTrait::TemperatureSetting, "availableThermostatModes=off,on,heat,cool;thermostatTemperatureUnit=C");

    //  show we are initialising
    oled.showSetup();

    //  give the impression of something happening
    delay(1000);

    //  set local ip address
    localIp = WiFi.localIP();

    //  load the options model
    EEPROM.get(0, options);
}

void checkLoop() {

    //  Always run the check events in the loop as we don't want to overheat the water
    checkEvents();

    //  do we need to override the running state
    if (isRunning && overrideOn) {

        isRunning = false;
    }
    else if (currentTemp < options.minTemp && overrideOn) {

        isRunning = true;
    }

    displayStatus(false);
    delay(250);
}

void startButtonPushed() {

    int buttonStateRead = digitalRead(BUTTON_PIN);

    if (buttonStateRead != 1) {

        return;
    }

    setOverrideState();
}

void setOverrideState() {

    if (overrideOn) {

        overrideOn = false;
    }
    else {

        overrideOn = true;
    }
}

void setRunningMode(bool running) {

    overrideOn = running;
}

/*
    Recevies a message from the Application
    Format deviceId;message

    returns 0 if handled, -1 if not
*/
int receiveMessage(String message) {

    Serial.printlnf("Message : %s", message.c_str());

    if (message == NULL || message.length() == 0) {

        return -1;
    }

    char buffer[message.length()];
    message.toCharArray(buffer, sizeof(buffer));

    //  first part of the message is the device UUID
    char* token = strtok(buffer, "^");
    String uuid = String(token);

    //  next we have the message itself
    token = strtok(NULL, "^");
    String msg = String(token);

    //  send this part back to any other devices
    String resend = String::format("%d;%s^%s", indexer, uuid.c_str(), msg.c_str());

    //  broadcast change message to clients so they have the opportunity to react
    Particle.publish("evt_regu", resend, 1, PRIVATE);

    //  Override button pressed
    if (msg.toLowerCase().startsWith("ovd")) {

        delay(3000);
        setOverrideState();
        return 0;
    }

    //  Restart Photon requested
    if (msg.toLowerCase().startsWith("rst")) {

        //  allow message to be sent
        delay(5000);

        System.reset();
        return 0;
    }

    /*
        Change to options

        The format of the message is opt[n];data

        We don't care about the first part of the message we just need to latter part
    */
    if (msg.toLowerCase().startsWith("opt")) {

        delay(3000);

        token = strtok(NULL, "^");
        String data = String(token);

        Serial.printlnf("data = %s", data.c_str());

        if (data.length() == 0) {

            return -1;
        }

        /*
            Data will be in the format of ; delimited strings
        */
        char buffer2[data.length()];
        data.toCharArray(buffer2, sizeof(buffer2));

        token = strtok(buffer2, ";");
        options.onStartHour = atoi(token);

        token = strtok(NULL, ";");
        options.onStartMin = atoi(token);

        token = strtok(NULL, ";");
        options.onEndHour = atoi(token);

        token = strtok(NULL, ";");
        options.onEndMin = atoi(token);

        token = strtok(NULL, ";");
        options.minTemp = atoi(token);

        token = strtok(NULL, ";");
        options.maxTemp = atoi(token);

        token = strtok(NULL, ";");
        options.lowBoundaryTemp = atoi(token);

        token = strtok(NULL, ";");
        options.highBoundaryTemp = atoi(token);

        //  finally save the options
        EEPROM.put(0, options);
        return 0;
    }

    return -1;
}
/*
    Check to see which events should be active
*/
void checkEvents() {

    isRunning = false;
    nightTimeMode = false;

    //  check to see if we should be running or not
    int hour = Time.hour();
    int minute = Time.minute();

    //  Check if the hour part of the time false within the specified overnight period
    if (hour > options.onStartHour && hour < options.onEndHour) {

        nightTimeMode = true;
        options.reachedMax = false;
        return;
    }

    if (hour == options.onStartHour && minute > options.onStartMin) {

        nightTimeMode = true;
        options.reachedMax = false;
        return;
    }

    if (hour == options.onEndHour && minute < options.onEndMin) {

        nightTimeMode = true;
        options.reachedMax = false;
        return;
    }

    /*
        The heating is on if any of the conditions are true:

            1 - Current Temperature is below the minimum the user has set for the water temperature
            2 - Current Temperature is between the minimum and maximum if the max temperature has not been achieved
    */
    if (currentTemp < options.minTemp) {

        isRunning = true;
        return;
    }

    if (currentTemp < options.maxTemp && !options.reachedMax) {

        isRunning = true;
        return;

    }

    //  if we have reached the maximum temperature then set the flag, it will only be reset when we hit nightime mode or the MaxTemp is changed
    if (currentTemp >= options.maxTemp) {

        options.reachedMax = true;
        return;
    }
}

/*
    Display the current status on screen
*/
void displayStatus(bool setOptions) {

    getTemperature();

    setGoogleData();

    String ls = String::format(
        "r=%d@sh=%d@sm=%d@eh=%d@em=%d@mn=%d@mx=%d@lbt=%d@hbt=%d@",
            isRunning,
            options.onStartHour,
            options.onStartMin,
            options.onEndHour,
            options.onEndMin,
            options.minTemp,
            options.maxTemp,
            options.lowBoundaryTemp,
            options.highBoundaryTemp);

    //  send change information if anything other than temperature has been changed
    if (ls != lastStatus) {

        prevTemp = currentTemp;
        lastSent = millis();
        lastStatus = ls;

        String msg = String::format("%d;%s;%s;%s@tmp=%.1f", indexer, version.c_str(), localIp.c_str(), lastStatus.c_str(), currentTemp);
        Particle.publish("evt_change", msg, 10, PRIVATE);
    }
    else {

        int current = millis() - lastSent;

        if (current >= 1000) {

            //  if current temperature is more than .02 different then send the update
            double variation = prevTemp - currentTemp;

            if (variation <= -0.2 || variation >= 0.2) {

                //  send just the temperature change
                lastSent = millis();
                prevTemp = currentTemp;

                Particle.publish("evt_temp", String::format("%d;%.1f", indexer, currentTemp), 10, PRIVATE);
            }
        }
    }

    if (nightTimeMode && !isRunning) {

        oled.displayCurrentTemperatureInActive(currentTemp);
    }
    else {

        String until = String::format("%02d:%02d", options.onStartHour, options.onStartMin);
        oled.displayTemperature(currentTemp, options.maxTemp, options.minTemp, isRunning, until);
    }
}

/*
    Set's the current temperature
*/
void getTemperature() {

    bool isValid = false;
    int attempts = 5;

    while(!isValid && attempts > 0) {

        //  read the current temperature in celcius
        dallas.requestTemperatures();

        currentTemp = dallas.getTempCByIndex(0);

        //  mis-read
        if (currentTemp <= -127.0f || currentTemp == 85) {

            delay(250);
            attempts--;
            continue;
        }

        isValid = true;
    }

    if (!isValid) {

        return;
    }
}

/*
    Handle the execute method
*/
int execute(std::vector<GoogleCommand>& deviceCommand) {

    DeviceState stateResult;
    stateResult.isFirstCommand = true;

    //  if a single command then it's easy
    if (deviceCommand.size() == 1) {

        stateResult = handleSingleCommand(deviceCommand[0], stateResult);
    }
    else {

        for (int i = 0; i < deviceCommand.size(); i++) {

            GoogleCommand cmd = deviceCommand[i];

            //  perform single command and see what else we need to do
            stateResult = handleSingleCommand(cmd, stateResult);

            switch(stateResult.result) {

                case ReturnCode::success: {

                    //  if command is processed successfully then just action then next
                    stateResult.isFirstCommand = false;
                    continue;
                }
                default: {

                    //  for now just return the result
                    Serial.printlnf("Command %d resulted in %d", cmd.deviceCommand, stateResult.result);
                    return (int)stateResult.result;
                }
            }
        }
    }

    if ((int)stateResult.result > 0) {

        //  update google data
        setGoogleData();
    }

    return (int)stateResult.result;
}

/*
    Handle a single command
*/
DeviceState handleSingleCommand(GoogleCommand command, DeviceState prevState) {

    DeviceState resultState;

    resultState.result = ReturnCode::unknownError;
    resultState.isCoolingCommand = false;

    switch(command.deviceCommand) {

        case DeviceCommand::ThermostatSetMode: {

            if (command.parameters.size() != 1) {

                resultState.result = ReturnCode::notSupported;
                return resultState;
            }

            GoogleParameter parm = command.parameters[0];

            //  The only parameter supported is thermostatMode, anything else return not supported
            if (parm.deviceParameter != DeviceParameter::thermostatMode) {

                resultState.result = ReturnCode::notSupported;
                return resultState;
            }

            String lcVal = parm.parameterValue.toLowerCase();

            //  turning the hot water on
            if (lcVal == "on" || lcVal == "heat") {

                //  can't turn on if we are above the maximum
                if (currentTemp >= (options.highBoundaryTemp + 2.0)) {

                  resultState.result = ReturnCode::valueOutOfRange;
                  return resultState;
                }

                //  running mode is the opposite of the state we want
                //  so on is false, off is true
                setRunningMode(false);

                resultState.result = ReturnCode::success;
                return resultState;
            }

            if (lcVal == "off") {

                setRunningMode(true);

                resultState.result = ReturnCode::success;
                return resultState;
            }

            //  if the value is cool may handle with multiple commands
            if (lcVal == "cool") {

                resultState.result = ReturnCode::success;
                resultState.isCoolingCommand = true;
                return resultState;
            }

            //  return not supported
            resultState.result = ReturnCode::notSupported;
            return resultState;
        }
        case DeviceCommand::ThermostatTemperatureSetpoint: {

            if (command.parameters.size() != 1) {

                resultState.result = ReturnCode::notSupported;
                return resultState;
            }

            GoogleParameter parm = command.parameters[0];

            //  the only parameter we accept here is thermostatTemperatureSetpoint
            if (parm.deviceParameter != DeviceParameter::thermostatTemperatureSetpoint) {

                resultState.result = ReturnCode::notSupported;
                return resultState;
            }

            //  check the incoming parameters
            int setting = atoi(parm.parameterValue);

            if (setting <= 0) {

                resultState.result = ReturnCode::valueOutOfRange;
                return resultState;
            }

            //  the value must be in between the bounary range
            if (setting < options.lowBoundaryTemp || setting > options.highBoundaryTemp) {

                resultState.result = ReturnCode::valueOutOfRange;
                return resultState;
            }

            //  if this is the first command or the command is for heat then set the upper boundary value
            if (prevState.isFirstCommand || !prevState.isCoolingCommand) {

                //  set the temperature
                options.maxTemp = setting;

                resultState.result = ReturnCode::success;
                return resultState;
            }

            //  if second command to set cooling then set the lower boundary value
            if (!prevState.isFirstCommand && prevState.isCoolingCommand) {

                //  set the temperature
                options.minTemp = setting;

                resultState.result = ReturnCode::success;
                return resultState;
            }

            resultState.result = ReturnCode::unknownError;
            return resultState;
        }
        default: {

            resultState.result = ReturnCode::notSupported;
            return resultState;
        }
    }

    resultState.result = ReturnCode::unknownError;
    return resultState;
}

/*
    Updates the temperature data
*/
void setGoogleData() {

    String mode = "off";

    //  update google temp data
    if (isRunning) {

      mode = "heat";
    }

    gHome.saveData(String::format("thermostatMode=%s;thermostatTemperatureSetpoint=%d;thermostatTemperatureAmbient=%.1f;",
      mode.c_str(),
      options.maxTemp,
      currentTemp));
}

/*
  Listen for request to send data to clients
*/
void publishStateListener(const char *event, const char *data) {

  delay(1000);
  String msg = String::format("%d;%s;%s;%s@tmp=%.1f", indexer, version.c_str(), localIp.c_str(), lastStatus.c_str(), currentTemp);
  Particle.publish("evt_change", msg, 10, PRIVATE);
}

/*
    Set's up the default time clock
*/
void setupDefaultClock() {

    EEPROM.clear();

    options.onStartHour = 1;
    options.onStartMin = 0;
    options.onEndHour = 6;
    options.onEndMin = 0;
    options.minTemp = 35;
    options.maxTemp = 55;
    options.lowBoundaryTemp = 10;
    options.highBoundaryTemp = 75;
    options.reachedMax = false;

    EEPROM.put(0, options);
}
