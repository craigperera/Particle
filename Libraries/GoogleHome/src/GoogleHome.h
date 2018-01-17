#pragma once

/* GoogleHome library by Craig Perera
 */

// This will load the definition for common Particle variable types
#include "Particle.h"
#include "GoogleHomeDefs.h"

typedef struct {

  DeviceTrait trait;
  String traitData;
} TraitInfoBlock;


// This is your main class that users will import into their application
class GoogleHome
{
public:
  /**
   * Constructor
   */
  GoogleHome(DeviceType deviceType);

  /**
   *  Resgister Supported Device Traits
   */
  void registerDeviceTrait(DeviceTrait trait, String traitData);

private:

};
