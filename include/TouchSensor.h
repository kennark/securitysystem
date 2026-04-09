#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include <Arduino.h>
#include "Config.h"
#include "EventQueue.h"

class EventQueue;  // Forward declaration
class TouchSensor {
public:
    TouchSensor();

    void begin();
    void update();

    void setEventQueue(EventQueue* queue) { eventQueuePtr = queue; };

    volatile bool touchWakeFlag = false;
private:
    EventQueue* eventQueuePtr;
};

extern TouchSensor* g_touchSensor;  // Global instance for ISR access

// ISR Handler - set wake flag (no queue access in ISR)
void IRAM_ATTR onTouchWake();

#endif