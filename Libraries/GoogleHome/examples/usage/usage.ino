// Example usage for GoogleHome library by Craig Perera.

#include "GoogleHome.h"

// Initialize objects from the lib
GoogleHome googleHome;

void setup() {
    // Call functions on initialized library objects that require hardware
    googleHome.begin();
}

void loop() {
    // Use the library's initialized objects and functions
    googleHome.process();
}
