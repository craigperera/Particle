#ifndef DEFINITIONS_H_INCLUDED
#define DEFINITIONS_H_INCLUDED

#include "application.h"

//  Constants
const int MACHINE_WASH = 1;
const int MACHINE_RINSE = 2;
const int MACHINE_SPIN = 4;
const int MACHINE_PUMPOUT = 8;
const int MACHINE_END = 16;
const int MACHINE_DOOR = 32;

const int READY = 0;
const int WASH_ENDED = 1;
const int WASHING = 2;
const int RINSING = 3;
const int SPINNING = 4;
const int EMPTYING = 5;

typedef struct {
    int a5;
    int a4;
    int a3;
    int a2;
    int a1;
    int a0;
    bool unset;
} Readings;

typedef struct {

    long cycleStarted;
    long washStarted;
    long washEnded;
    long rinseStarted;
    long rinseEnded;
    long spinStarted;
    long spinEnded;
    long pumpoutStarted;
    long pumpoutEnded;
    long cycleEnded;

} WashTimings;

extern long lastNotified;
extern String version;
extern WashTimings currentWash;
extern String lastMessageSent;
extern String particleName;
extern bool isStarting;

#endif
