/* GoogleHome library by Craig Perera
 */
#include "application.h"
#include "GoogleHome.h"
#include "GoogleHomeDefs.h"

#include <vector>

DeviceTypes googleHomeType;
std::vector<TraitInfoBlock> deviceTraits(0);

int getDeviceInformation(String request);

//  variables
String gData;

/**
 * Constructor.
 */
GoogleHome::GoogleHome(DeviceTypes deviceType)
{
  googleHomeType = deviceType;

  //  clear data items
  gData = "";

  //  register required varaibles
  Particle.variable("gData", gData);

  // register required functions
  Particle.function("gInfo", getDeviceInformation);
}

/*
  Register device traits
*/
void GoogleHome::registerDeviceTrait(DeviceTraits trait, String traitData) {

  TraitInfoBlock tBlock;

  tBlock.trait = trait;
  tBlock.traitData = traitData;

  deviceTraits.push_back(tBlock);
}


/**
 * Example method.
 */
void GoogleHome::process()
{
    // do something useful
    Serial.println("called process");
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
    int ho1 = googleHomeType << 24;
    int ho2 = traitCount <<16;

    //  set the result into gData
    gData = tBlock.traitData;
    return ho1 | ho2 | tBlock.trait;
  }

  return -1;
}
