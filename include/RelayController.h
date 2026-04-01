#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

class RelayController {
public:
    RelayController(int pin = PIN_RELAY);
    
    void begin();
    
    // Power control
    void enablePower();
    void disablePower();
    
    // Status
    bool isPowered() const { return powered; }
    
private:
    int relayPin;
    bool powered;
};

#endif // RELAY_CONTROLLER_H