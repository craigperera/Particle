/* GoogleHome library by Craig Perera
 */

#include "GoogleHome.h"

/**
 * Constructor.
 */
GoogleHome::GoogleHome()
{
  // be sure not to call anything that requires hardware be initialized here, put those in begin()
}

/**
 * Example method.
 */
void GoogleHome::begin()
{
    // initialize hardware
    Serial.println("called begin");
}

/**
 * Example method.
 */
void GoogleHome::process()
{
    // do something useful
    Serial.println("called process");
    doit();
}

/**
* Example private method
*/
void GoogleHome::doit()
{
    Serial.println("called doit");
}
