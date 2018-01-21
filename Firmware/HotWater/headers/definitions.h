#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "application.h"
#include "oledcontrol.h"

#include <vector>
#include <GoogleHomeDefs.h>

typedef struct {

    int hour;
    int minute;

} EventTime;

typedef struct {

    int  onStartHour;
    int  onStartMin;
    int  onEndHour;
    int  onEndMin;
    int  minTemp;
    int  maxTemp;
    int  lowBoundaryTemp;
    int  highBoundaryTemp;
    bool reachedMax;

} OptionsModel;

typedef struct {

    ReturnCode result;
    bool isCoolingCommand;
    bool isFirstCommand;

} DeviceState;


//  externals
extern bool isStarting;
extern long lastSent;
extern String lastStatus;

extern String version;
extern String localIp;

extern OptionsModel options;
extern oledcontrol oled;

#endif
