#include "application.h"
#include "../headers/definitions.h"
#include "../headers/testprogram.h"
#include "../headers/code.h"

//  Internal Methods
void updateWashStatus();
void initState();
void initStateOnStartup();
void setPanelState(Readings state);
void setCurrentState();

Readings loadPinState();

//  particle functions
void publishMessage(int machineState, bool endWashNotification);
void publishStateListener(const char *event, const char *data);
int receiveMessage(String message);

//  constants
const int indexer = 1;

//  variables
String version = "WMF_0.0.5a";

//  Washing Control
String lastMessageSent;

String localIp = "";

int washStatus = READY;

//  Washing running flag
bool IsWashing = false;

WashTimings currentWash;

long washingFinished = 0;

/*
    Initilise the particle on startup
*/
void initialise() {

  //  load the options model
  setADCSampleTime(ADC_SampleTime_3Cycles);

  //  set local ip address
  localIp = WiFi.localIP();

  //  Register functions
  Particle.function("runTest", setTestProgram);
  Particle.function("pMessage", receiveMessage);

  //  register listener
  Particle.subscribe("publishData", publishStateListener);

  //  now setup for running
  initStateOnStartup();
}

/*
  Main loop to check current status
*/
void checkLoop() {

  Readings current = loadTestPinState();

  //  readings not set by the test program, then load from real chip
  if (current.unset) {

    current = loadPinState();
  }

  setPanelState(current);
}

/*
  Initialise the state object when washing begins
*/
void initStateOnStartup() {

  //  load the options model
  WashTimings timings;
  EEPROM.get(0, timings);

  if (timings.cycleStarted > -1) {

    currentWash.cycleStarted = timings.cycleStarted;
    currentWash.washStarted = timings.washStarted;
    currentWash.washEnded = timings.washEnded;
    currentWash.rinseStarted = timings.rinseStarted;
    currentWash.rinseEnded = timings.rinseEnded;
    currentWash.spinStarted = timings.spinStarted;
    currentWash.spinEnded = timings.spinEnded;
    currentWash.pumpoutStarted = timings.pumpoutStarted;
    currentWash.pumpoutEnded = timings.pumpoutEnded;
    currentWash.cycleEnded = timings.cycleEnded;
  }
  else {

    currentWash.cycleStarted = 0;
    currentWash.washStarted = 0;
    currentWash.washEnded = 0;
    currentWash.rinseStarted = 0;
    currentWash.rinseEnded = 0;
    currentWash.spinStarted = 0;
    currentWash.spinEnded = 0;
    currentWash.pumpoutStarted = 0;
    currentWash.pumpoutEnded = 0;
    currentWash.cycleEnded = 0;

    EEPROM.put(0, currentWash);
  }

  //  setup last Message
  lastMessageSent = String::format("%d@%d@%d@%d@%d@%d@%d@%d@%d@%d@%d@",
      currentWash.cycleStarted,
      (currentWash.washStarted > 0) ? currentWash.washStarted - currentWash.cycleStarted : 0,
      (currentWash.washEnded > 0) ? currentWash.washEnded - currentWash.cycleStarted : 0,
      (currentWash.rinseStarted > 0) ? currentWash.rinseStarted - currentWash.cycleStarted : 0,
      (currentWash.rinseEnded > 0) ? currentWash.rinseEnded - currentWash.cycleStarted : 0,
      (currentWash.spinStarted > 0) ? currentWash.spinStarted - currentWash.cycleStarted : 0,
      (currentWash.spinEnded > 0) ? currentWash.spinEnded - currentWash.cycleStarted : 0,
      (currentWash.pumpoutStarted > 0) ? currentWash.pumpoutStarted - currentWash.cycleStarted : 0,
      (currentWash.pumpoutEnded > 0) ? currentWash.pumpoutEnded - currentWash.cycleStarted : 0,
      (currentWash.cycleEnded > 0) ? currentWash.cycleEnded - currentWash.cycleStarted : 0,
      READY);

  //  to ensure we clear pending message always broadcast details on startup
  String msg = String::format("%d;%s;%s;%s", indexer, version.c_str(), localIp.c_str(), lastMessageSent.c_str());
  Particle.publish("evt_change", msg, 10, PRIVATE);
}

/*
    Initialise State Objects
*/
void initState() {

  currentWash.cycleStarted = 0;
  currentWash.washStarted = 0;
  currentWash.washEnded = 0;
  currentWash.rinseStarted = 0;
  currentWash.rinseEnded = 0;
  currentWash.spinStarted = 0;
  currentWash.spinEnded = 0;
  currentWash.pumpoutStarted = 0;
  currentWash.pumpoutEnded = 0;
  currentWash.cycleEnded = 0;

  EEPROM.put(0, currentWash);
}

/*
    Load the pin states from the Particle
*/
Readings loadPinState() {

  Readings readings;

  readings.a5 = 0;
  readings.a4 = 0;
  readings.a3 = 0;
  readings.a2 = 0;
  readings.a1 = 0;
  readings.a0 = 0;

  for (int i = 0; i < 5; i++) {

    int reading = analogRead(A0);

    if (reading > readings.a0) {

      readings.a0 = reading;
    }

    reading = analogRead(A5);

    if (reading > readings.a5) {

      readings.a5 = reading;
    }

    reading = analogRead(A1);

    if (reading > readings.a1) {

      readings.a1 = reading;
    }

    reading = analogRead(A4);

    if (reading > readings.a4) {

      readings.a4 = reading;
    }

    reading = analogRead(A2);

    if (reading > readings.a2) {

      readings.a2 = reading;
    }

    reading = analogRead(A3);

    if (reading > readings.a3) {

      readings.a3 = reading;
    }

    delay(25);
  }

  readings.unset = false;
  return readings;
}

/*
    Setup the machine state flag from our readings
*/
void setPanelState(Readings state) {

  bool wash = (state.a5 > 300) ? true : false;
  bool rinse = (state.a4 > 300) ? true : false;
  bool spin = (state.a3 > 300) ? true : false;
  bool empty = (state.a2 > 300) ? true : false;
  bool machineEnd = (state.a1 > 300) ? true : false;
  bool doorLock = (state.a0 > 1000) ? true : false;

  //  how many panel lights are on ?
  int count = 0;

  if (wash) {

      count++;
  }

  if (rinse) {

      count++;
  }

  if (spin) {

      count++;
  }

  if (empty) {

      count++;
  }

  /*
    Cycle is ended when the door lock is off when we were in a washing state
    also the machine end light has gone on
  */
  if (IsWashing && !doorLock && machineEnd && count == 0) {

    if (currentWash.cycleStarted > 0) {

      if (currentWash.washStarted > 0 && currentWash.washEnded == 0) {

        currentWash.washEnded = Time.now();
      }

      if (currentWash.rinseStarted > 0 && currentWash.rinseEnded == 0) {

        currentWash.rinseEnded = Time.now();
      }

      if (currentWash.spinStarted > 0 && currentWash.spinEnded == 0) {

        currentWash.spinEnded = Time.now();
      }

      if (currentWash.pumpoutStarted > 0 && currentWash.pumpoutEnded == 0) {

        currentWash.pumpoutEnded = Time.now();
      }

      currentWash.cycleEnded = Time.now();
    }

    washingFinished = millis();

    publishMessage(WASH_ENDED, true);
    IsWashing = false;
    return;
  }

  //  if machine is off, or not running then no further action required
  if (!wash && !rinse && !spin && !empty) {

    if (washStatus == WASH_ENDED || lastMessageSent.endsWith("1@")) {

      IsWashing = false;

      int current = millis() - washingFinished;

      //  reset after 5 minutes of end
      if (current >= 300000) {

        washingFinished = 0;
        washStatus = READY;
        publishMessage(READY, false);
      }
    }

    return;
  }

  //  Machine is currently Washing
  if (wash && doorLock && count == 1) {

    if (currentWash.cycleStarted > 0 && currentWash.cycleEnded > 0) {

      initState();
    }

    if (currentWash.cycleStarted == 0) {

      currentWash.cycleStarted = Time.now();
      currentWash.cycleStarted -= 1;
    }

    if (currentWash.washStarted == 0) {

      currentWash.washStarted = Time.now();
    }

    publishMessage(WASHING, false);
    IsWashing = true;
    return;
  }

  //  Machine is currently Rinsing
  if (rinse && doorLock && count == 1) {

    if (currentWash.cycleStarted > 0 && currentWash.cycleEnded > 0) {

      initState();
    }

    if (currentWash.cycleStarted == 0) {

      currentWash.cycleStarted = Time.now();
      currentWash.cycleStarted -= 1;
    }

    if (currentWash.rinseStarted == 0) {

      currentWash.rinseStarted = Time.now();
    }

    if (currentWash.washStarted > 0 && currentWash.washEnded == 0) {

      currentWash.washEnded = Time.now();
    }

    publishMessage(RINSING, false);
    IsWashing = true;
    return;
  }

  //  Machine is currently Rinsing
  if (spin && doorLock && count == 1) {

    if (currentWash.cycleStarted > 0 && currentWash.cycleEnded > 0) {

      initState();
    }

    if (currentWash.cycleStarted == 0) {

      currentWash.cycleStarted = Time.now();
      currentWash.cycleStarted -= 1;
    }

    if (currentWash.spinStarted == 0) {

      currentWash.spinStarted = Time.now();
    }

    if (currentWash.washStarted > 0 && currentWash.washEnded == 0) {

      currentWash.washEnded = Time.now();
    }

    if (currentWash.rinseStarted > 0 && currentWash.rinseEnded == 0) {

      currentWash.rinseEnded = Time.now();
    }

    publishMessage(SPINNING, false);
    IsWashing = true;
    return;
  }

  //  Machine is currently Rinsing
  if (empty && doorLock && count == 1) {

    if (currentWash.cycleStarted > 0 && currentWash.cycleEnded > 0) {

      initState();
    }

    if (currentWash.cycleStarted == 0) {

      currentWash.cycleStarted = Time.now();
      currentWash.cycleStarted -= 1;
    }

    if (currentWash.pumpoutStarted == 0) {

      currentWash.pumpoutStarted = Time.now();
    }

    if (currentWash.washStarted > 0 && currentWash.washEnded == 0) {

      currentWash.washEnded = Time.now();
    }

    if (currentWash.rinseStarted > 0 && currentWash.rinseEnded == 0) {

      currentWash.rinseEnded = Time.now();
    }

    if (currentWash.spinStarted > 0 && currentWash.spinEnded == 0) {

      currentWash.spinEnded = Time.now();
    }

    publishMessage(EMPTYING, false);
    IsWashing = true;
    return;
  }
}

/*
    Publish Message to any connected clients
*/
void publishMessage(int machineState, bool endWashNotification) {

  String tMessage = String::format("%d@%d@%d@%d@%d@%d@%d@%d@%d@%d@%d@",
    currentWash.cycleStarted,
    (currentWash.washStarted > 0) ? currentWash.washStarted - currentWash.cycleStarted : 0,
    (currentWash.washEnded > 0) ? currentWash.washEnded - currentWash.cycleStarted : 0,
    (currentWash.rinseStarted > 0) ? currentWash.rinseStarted - currentWash.cycleStarted : 0,
    (currentWash.rinseEnded > 0) ? currentWash.rinseEnded - currentWash.cycleStarted : 0,
    (currentWash.spinStarted > 0) ? currentWash.spinStarted - currentWash.cycleStarted : 0,
    (currentWash.spinEnded > 0) ? currentWash.spinEnded - currentWash.cycleStarted : 0,
    (currentWash.pumpoutStarted > 0) ? currentWash.pumpoutStarted - currentWash.cycleStarted : 0,
    (currentWash.pumpoutEnded > 0) ? currentWash.pumpoutEnded - currentWash.cycleStarted : 0,
    (currentWash.cycleEnded > 0) ? currentWash.cycleEnded - currentWash.cycleStarted : 0,
    machineState);

  if (machineState == 0) {

    washStatus = READY;
  }
  else {

    washStatus = machineState;
  }

  if (endWashNotification && IsWashing) {

    //  Send IOS Notification
    Particle.publish("foregroundNotification", "The Washing is ready to go into the dryer", PRIVATE);

    lastMessageSent = tMessage;
    IsWashing = false;

    //  store in EEPROM
    EEPROM.put(0, currentWash);

    Particle.publish("evt_change", String::format("%d;%s;%s;%s;", indexer, version.c_str(), localIp.c_str(), lastMessageSent.c_str()), 10, PRIVATE);
    return;
  }

  if (lastMessageSent != tMessage) {

    lastMessageSent = tMessage;

    //  store in EEPROM
    EEPROM.put(0, currentWash);

    Particle.publish("evt_change", String::format("%d;%s;%s;%s;", indexer, version.c_str(), localIp.c_str(), lastMessageSent.c_str()), 10, PRIVATE);
  }
}

/*
  Listen for request to send data to clients
*/
void publishStateListener(const char *event, const char *data) {

  delay(1500);
  String msg = String::format("%d;%s;%s;%s", indexer, version.c_str(), localIp.c_str(), lastMessageSent.c_str());
  Particle.publish("evt_change", msg, 10, PRIVATE);
}

/*
  Recevies a message from the Application
  Format deviceId;message

  returns 0 if handled, -1 if not
*/
int receiveMessage(String message) {

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

  //  Restart Photon requested
  if (msg.toLowerCase().startsWith("rst")) {

    //  save state
    EEPROM.put(0, currentWash);

    Particle.publish("RESTARTING");

    //  allow message to be sent
    delay(2000);

    System.reset();
    return 0;
  }

  //  undefined message
  return -1;
}
