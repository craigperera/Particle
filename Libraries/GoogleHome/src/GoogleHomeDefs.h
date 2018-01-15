#pragma once

typedef enum {
  Camera = 1,
  Dishwasher = 2,
  Dryer = 3,
  Light = 4,
  Outlet = 5,
  Switch = 6,
  Thermostat = 7,
  Vaccuum = 8,
  Washer = 9
} DeviceTypes;

typedef enum {

  Brightness = 1,
  CameraStream = 2,
  ColorSpectrum = 4,
  ColorTemperature = 8,
  Dock = 16,
  Modes = 32,
  OnOff = 64,
  RunCycle = 128,
  Scenes = 256,
  StartStop = 512,
  TemperatureSetting = 1024,
  Toggles = 2048
} DeviceTraits;

typedef enum {

    unknownError = -1,
    authExpired = -2,
    authFailure = -3,
    deviceOffline = -4,
    timeout = -5,
    deviceTurnedOff = -6,
    deviceNotFound = -7,
    valueOutOfRange = 8,
    notSupported = -9,
    protocolError = -10,
    resourceUnavailable = -100,
    inHeatOrCool = -200,
    inHeatCool = -201,
    lockedToRange = -202,
    rangeTooClose = -203

} ErrorCode;
