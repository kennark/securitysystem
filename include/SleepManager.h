#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "EventQueue.h"
#include "SecurityTypes.h"

class SleepManager {
public:
    SleepManager();
    
    // Configure wake sources and sleep parameters
    void begin();
    
    // Check idle conditions and enter sleep if appropriate
    void update();
    
    // Execute light sleep immediately
    void enterSleep();
    
    // Dependency injection for event queue and activity timestamp
    void setEventQueue(EventQueue* queue) { eventQueue = queue; };
    void setLastActionTime(unsigned long* lastActionPtr) { lastActionTime = lastActionPtr; };
    void setSleepTimeout(uint32_t* timeout) { sleepTimeout = timeout; };

private:
    // Dependencies
    EventQueue* eventQueue;
    unsigned long* lastActionTime;
    uint32_t* sleepTimeout;
    
    // Helper Methods
    bool isIdleTimeExceeded() const;
    bool canEnterSleep() const;
};

#endif