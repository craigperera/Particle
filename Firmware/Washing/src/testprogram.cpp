#include "application.h"
#include "../headers/definitions.h"

bool runTestProgram = false;
int lastEvent = 0;

const int SECONDS = 1000;
const int MINUTES = SECONDS * 60;
const int HOURS = MINUTES * 60;

const int TURN_ON = SECONDS * 5;
const int MACHINE_ON_NOT_LOCKED = TURN_ON + (SECONDS * 4);
const int START_WASHING = MACHINE_ON_NOT_LOCKED + SECONDS;
const int START_RINSING = START_WASHING + (SECONDS * 25);
const int START_SPINNING = START_RINSING + (SECONDS * 15);
const int START_PUMPING = START_SPINNING + (SECONDS * 35);
const int PUMPOUT_ENDED = START_PUMPING + (SECONDS * 15);
const int WASHING_ENDED = PUMPOUT_ENDED + (SECONDS * 1);
const int TURN_OFF = WASHING_ENDED + (SECONDS * 5);

Readings loadTestPinState() {

  Readings readings;

  readings.a5 = 0;
  readings.a4 = 0;
  readings.a3 = 0;
  readings.a2 = 0;
  readings.a1 = 0;
  readings.a0 = 0;
  readings.unset = true;

  if (!runTestProgram) {

    return readings;
  }

  readings.unset = false;

  //  if not set then we are just starting
  if (lastEvent == 0) {

    lastEvent = millis();
    return readings;
  }

  //  what phase are we on
  int current = millis() - lastEvent;

  if (current >= TURN_ON && current < START_WASHING) {

    readings.a5 = 500;
    readings.a4 = 500;
    readings.a3 = 500;
    readings.a2 = 500;
    readings.a1 = 0;
    readings.a0 = 2500;

    return readings;
  }

  if (current >= MACHINE_ON_NOT_LOCKED && current < START_WASHING) {

    readings.a5 = 500;
    readings.a0 = 234;
    return readings;
  }

  if (current >= START_WASHING && current < START_RINSING) {

    readings.a5 = 500;
    readings.a0 = 2500;
    return readings;
  }

  if (current >= START_RINSING && current < START_SPINNING) {

    readings.a4 = 500;
    readings.a0 = 2500;

    return readings;
  }

  if (current >= START_SPINNING && current < START_PUMPING) {

    readings.a3 = 500;
    readings.a0 = 2500;
    return readings;
  }

  if (current >= START_PUMPING && current < PUMPOUT_ENDED) {

    readings.a2 = 500;
    readings.a0 = 2500;
    return readings;
  }

  if (current >= PUMPOUT_ENDED && current < WASHING_ENDED) {

    readings.a5 = 206;
    readings.a4 = 1;
    readings.a3 = 1;
    readings.a2 = 8;
    readings.a1 = 1;
    readings.a0 = 3711;
    return readings;
  }

  if (current >= WASHING_ENDED && current < TURN_OFF) {

    readings.a5 = 0;
    readings.a4 = 0;
    readings.a3 = 1;
    readings.a2 = 8;
    readings.a1 = 500;
    readings.a0 = 0;

    runTestProgram = false;
    lastEvent = 0;

    return readings;
  }

  return readings;
}

int setTestProgram(String command) {

  if (command == "start") {

    if (runTestProgram) {

      return 0;
    }

    lastEvent = millis();
    runTestProgram = true;
  }
  else {

    lastEvent = 0;
    runTestProgram = false;
  }

  return 0;
}
