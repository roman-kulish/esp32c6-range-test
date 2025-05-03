#include "role.h"

Role::Role(Protocol* protocol, GPSHandler* gpsHandler)
    : protocol(protocol), gpsHandler(gpsHandler), initialized(false) {
}

Role::~Role() {
    // Base class destructor
}