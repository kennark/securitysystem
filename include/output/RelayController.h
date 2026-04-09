#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

class RelayController {
public:
    RelayController();
    
    void begin();
    void setRelayStatePtr(bool* ptr) { relayStatePtr = ptr; }
    
    // Power control
    void enablePower();
    void disablePower();
    
    // Status
    bool isPowered() const { return relayStatePtr ? *relayStatePtr : false; }
    
private:
    bool* relayStatePtr;
};

#endif // RELAY_CONTROLLER_H