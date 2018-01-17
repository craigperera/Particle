/* GoogleHome library by Craig Perera
 */
#include "application.h"
#include "GoogleHome.h"
#include "GoogleHomeDefs.h"

#include <vector>

DeviceType googleHomeType;
std::vector<TraitInfoBlock> deviceTraits(0);

int getDeviceInformation(String request);
int executeCommand(String command);
int translateIncomingCommand();

String getDeviceCommandName(int commandInteger);
String getParameterName(int parameterInteger);


//  variables
String gData;
String incomingData = "";

/**
 * Constructor.
 */
GoogleHome::GoogleHome(DeviceType deviceType)
{
  googleHomeType = deviceType;

  //  clear data items
  gData = "";

  //  register required varaibles
  Particle.variable("gData", gData);

  // register required functions
  Particle.function("gInfo", getDeviceInformation);
  Particle.function("gExecute", executeCommand);
}

/*
  Register device traits
*/
void GoogleHome::registerDeviceTrait(DeviceTrait trait, String traitData) {

  TraitInfoBlock tBlock;

  tBlock.trait = trait;
  tBlock.traitData = traitData;

  deviceTraits.push_back(tBlock);
}

/*
  Called during Google Home Registration to establish deviceTraits
  Type and Capabilities

  returns integer in the format:

    First High order byte = Device Type
    Second High order byte = Trait Count

    First Low Order byte = 1 or 0 where 1 indicates the trait has attributes and 0 indicates no attributes
    Second low order byte is the trait numerical value
*/
int getDeviceInformation(String request)
{
  if (request == NULL || request.length() == 0) {

    return - 1;
  }

  //  handle trait request
  if (request.toLowerCase().startsWith("traits")) {

    //  get the index
    request.replace("traits;", "");
    int index = atoi(request);

    if (index > deviceTraits.size()) {

      return -1;
    }

    int traitCount = deviceTraits.size();

    //  get the device trait
    TraitInfoBlock tBlock = deviceTraits[index];

    //  first call so return the device type in the high order byte, the trait count in the second high order byte
    int ho1 = (int)googleHomeType << 24;
    int ho2 = traitCount << 16;

    //  set the result into gData
    gData = tBlock.traitData;
    return ho1 | ho2 | (int)tBlock.trait;
  }

  return -1;
}

/*
    Handle an exec send from Google Home

    This method will be called one or more times to pass the command from google.

    If the whole command is < 63 bytes we will be called 1 time,
    Each call made will be to send the command or part of command
    to allow for 63 bit minimum

    Each time we are called we get the following:

    [total];[index];[data]

    where [total] is the number of parts to be sent in total,
    [index] is the current part number and [data] is the part of the
    command being sent
*/
int executeCommand(String command) {

  // first split into tokens
  char buffer[command.length()];
  command.toCharArray(buffer, sizeof(buffer));

  //  first get the total
  char* token = strtok(buffer, ";");
  int total = atoi(token);

  //  next the index
  token = strtok(NULL, ";");
  int index = atoi(token);

  //  finally the data
  token = strtok(NULL, ";");
  String data = String(token);

  //  if this is the first call then overwrite the local variable
  if (index == 1) {

      incomingData = data;
      return 1;
  }

  incomingData = incomingData + data;

  //  more data to come ?
  if (index < total) {

      return 1;
  }

  //  now we have the full string we can parse it into commands
  return translateIncomingCommand();
}

/*
    Turns the incoming command into verbose commands

    i.e. 12^13^cool will change to ThermostatSetMode;thermostatMode=cool
*/
int translateIncomingCommand() {

    if (incomingData == NULL || incomingData.length() == 0) {

        return -1;
    }

    std::vector<String> inCommands(0);

    //  first seperate the commands by , to get each command
    char buffer[incomingData.length()];
    incomingData.toCharArray(buffer, sizeof(buffer));

    String translatedCommand = "";

    //  first get the total
    char* token = strtok(buffer, ",");

    //  we can only run one strtok at a time so extract commands to the vector
    while(token != NULL) {

        inCommands.push_back(String(token));
        token = strtok(NULL, ",");
    }

    //  now we can process each command
    std::vector<GoogleCommand> deviceCommands(0);

    for (int i = 0; i < inCommands.size(); i++) {

        //  first get the command from the stack
        String command = inCommands[i];

        // now create the structure
        GoogleCommand googleCommand;

        //  command is in the format of Command^ActionName^ActionValue
        char buffer2[command.length()];
        command.toCharArray(buffer2, sizeof(buffer2));

        //  first is the device command
        char* arg = strtok(buffer2, "^");
        googleCommand.deviceCommand = getDeviceCommandName(atoi(arg));

        //  next get the parameter name
        arg = strtok(NULL, "^");
        googleCommand.deviceParameter = getParameterName(atoi(arg));

        // finally the command parameter value
        arg = strtok(NULL, "^");

        googleCommand.parameterValue = String(arg);
        deviceCommands.push_back(googleCommand);
    }

    inCommands.clear();

    //  invoke the actual function and return it's result
    return execute(deviceCommands);
}

/*
    Translate the Command Integer into a device command
*/
String getDeviceCommandName(int iCommandInteger) {

    if (iCommandInteger <= 0) {

        return "";
    }

    DeviceCommand commandInteger = (DeviceCommand)iCommandInteger;

    switch(commandInteger) {

        case DeviceCommand::BrightnessAbsolute: {

            return "BrightnessAbsolute";
        }
        case DeviceCommand::GetCameraStream: {

            return "GetCameraStream";
        }
        case DeviceCommand::ColorAbsolute: {

            return "ColorAbsolute";
        }
        case DeviceCommand::Dock: {

            return "Dock";
        }
        case DeviceCommand::SetModes: {

            return "SetModes";
        }
        case DeviceCommand::OnOff: {

            return "OnOff";
        }
        case DeviceCommand::ActivateScene: {

            return "ActivateScene";
        }
        case DeviceCommand::StartStop: {

            return "StartStop";
        }
        case DeviceCommand::PauseUnpause: {

            return "PauseUnpause";
        }
        case DeviceCommand::ThermostatTemperatureSetpoint: {

            return "ThermostatTemperatureSetpoint";
        }
        case DeviceCommand::ThermostatTemperatureSetRange: {

            return "ThermostatTemperatureSetRange";
        }
        case DeviceCommand::ThermostatSetMode: {

            return "ThermostatSetMode";
        }
        case DeviceCommand::SetToggle: {

            return "SetToggle";
        }
        default: {

            return "";
        }
    }
}

/*
    Translate the Paramater Name Integer into a device parameter
*/
String getParameterName(int iParameterInteger) {

    if (iParameterInteger <= 0) {

        return "";
    }

    DeviceParameter parameterInteger = (DeviceParameter)iParameterInteger;

    switch(parameterInteger) {

        case DeviceParameter::brightness: {

            return "brightness";
        }
        case DeviceParameter::StreamToChromecast: {

            return "StreamToChromecast";
        }
        case DeviceParameter::SupportedStreamProtocols: {

            return "SupportedStreamProtocols";
        }
        case DeviceParameter::color: {

            return "color";
        }
        case DeviceParameter::updateModeSettings: {

            return "updateModeSettings";
        }
        case DeviceParameter::on: {

            return "on";
        }
        case DeviceParameter::deactivate: {

            return "deactivate";
        }
        case DeviceParameter::start: {

            return "start";
        }
        case DeviceParameter::pause: {

            return "pause";
        }
        case DeviceParameter::thermostatTemperatureSetpoint: {

            return "thermostatTemperatureSetpoint";
        }
        case DeviceParameter::thermostatTemperatureSetpointHigh: {

            return "thermostatTemperatureSetpointHigh";
        }
        case DeviceParameter::thermostatTemperatureSetpointLow: {

            return "thermostatTemperatureSetpointLow";
        }
        case DeviceParameter::thermostatMode: {

            return "thermostatMode";
        }
        case DeviceParameter::updateToggleSettings: {

            return "updateToggleSettings";
        }
        default: {

            return "";
        }
    }
}
