#pragma once

#include <vector>

typedef enum class DeviceType {
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

typedef enum class DeviceTrait {

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

typedef enum class DeviceCommand {

  BrightnessAbsolute = 1,
  GetCameraStream = 2,
  ColorAbsolute = 3,
  Dock = 4,
  SetModes = 5,
  OnOff = 6,
  ActivateScene = 7,
  StartStop = 8,
  PauseUnpause = 9,
  ThermostatTemperatureSetpoint = 10,
  ThermostatTemperatureSetRange = 11,
  ThermostatSetMode = 12,
  SetToggle = 13
} DeviceCommands;

typedef enum class DeviceParameter {

  brightness = 1,
  StreamToChromecast = 2,
  SupportedStreamProtocols = 3,
  color = 4,
  updateModeSettings = 5,
  on = 6,
  deactivate = 7,
  start = 8,
  pause = 9,
  thermostatTemperatureSetpoint = 10,
  thermostatTemperatureSetpointHigh = 11,
  thermostatTemperatureSetpointLow = 12,
  thermostatMode = 13,
  updateToggleSettings = 14
} DeviceParameters;

typedef enum class ReturnCode {

    success = 1,
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
    rangeTooClose = -203,
    handleWithMultipleCommands = -999
} ReturnCodes;

typedef struct {

    DeviceCommand deviceCommand;
    DeviceParameter deviceParameter;
    String parameterValue;

} GoogleCommand;

//  external functions
extern int execute(std::vector<GoogleCommand>& deviceCommand);

//  external variables
extern String gData;
