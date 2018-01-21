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

//  variables
String gData;
String gTraitData;
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
  Particle.variable("gTraits", gTraitData);

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
void GoogleHome::saveData(String dataToSave) {

  gData = dataToSave;
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
    gTraitData = tBlock.traitData;
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
  }
  else {

    incomingData = incomingData + data;
  }

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
        googleCommand.parameters = std::vector<GoogleParameter>(0);

        /*
          Command is in the format of Command^[parameters]

          First get the command name
        */
        char buffer2[command.length()];
        command.toCharArray(buffer2, sizeof(buffer2));

        char* arg = strtok(buffer2, "^");
        googleCommand.deviceCommand = (DeviceCommand) atoi(arg);

        /*
          Parameters are a list of Key value pairs seperated with ^

          i.e. if the incoming data was Command^set=cool^set=warm

          this translated to Command with 2 parameters set to cool, then set to warm
        */

        while (true) {

          arg = strtok(NULL, "^");

          if (arg == NULL) {

            break;
          }

          //  we have the parameter item so find the equals sign
          String kvp = String(arg);

          int eqPos = kvp.indexOf('=');

          if (eqPos == -1) {

              continue;
          }

          String lft = kvp.substring(0, eqPos);
          String rgt = kvp.substring(eqPos + 1, kvp.length());

          GoogleParameter parm;
          parm.deviceParameter = (DeviceParameter)atoi(kvp.substring(0, eqPos));
          parm.parameterValue = kvp.substring(eqPos + 1, kvp.length());

          Serial.printlnf("Found parm %d with value %s", parm.deviceParameter, parm.parameterValue.c_str());
          googleCommand.parameters.push_back(parm);
        }

/*
        //  next get the parameter name
        arg = strtok(NULL, "^");
        googleCommand.deviceParameter = (DeviceParameter)atoi(arg);

        // finally the command parameter value
        arg = strtok(NULL, "^");

        googleCommand.parameterValue = String(arg);*/
        deviceCommands.push_back(googleCommand);
    }

    inCommands.clear();

    //  invoke the actual function and return it's result
    return execute(deviceCommands);
}
