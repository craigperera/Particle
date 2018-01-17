// Example usage for GoogleHome library by Craig Perera.
//#include "../src/GoogleHome.h"
//#include "../src/GoogleHomeDefs.h"

/*
// Initialize objects from the lib
GoogleHome googleHome(DeviceType::Thermostat);

void setup() {

  //  register device Traits
  googleHome.registerDeviceTrait(DeviceTrait::TemperatureSetting, "availableThermostatModes=off,on,heat,cool;thermostatTemperatureUnit=C");
}

/*
  You must implement the execute method somewhere in the code to handle the command sent from Google

  It will be received and parsed by the GoogleHome library class, formatted and sent as a vector
  of GoogleCommand structures, 1 for each command sent from google.

  Return a value > 0 to indicate success or a valid error code to indicate failure
*/
/*
int execute(std::vector<GoogleCommand>& deviceCommand) {

  /*
    Respond to commands here
  */
  /*
  for (int i = 0; i < deviceCommand.size(); i++) {

    GoogleCommand cmd = deviceCommand[i];

    //  for simple example just print the commands
    Serial.printlnf("cmd = %s, parm = %s, val = %s", cmd.deviceCommand.c_str(), cmd.deviceParameter.c_str(), cmd.parameterValue.c_str());
  }
  */

  /*
    On completion of work set the status values in gData and return > 0

    When > 0 is received by the call to the particle we retrieve the value in gData and attach that to the response
    back to google home
  */
  //  Set gData
  //  i.e. gData = ""
//    return -1;
//}
