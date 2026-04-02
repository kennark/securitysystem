#ifndef RF_RECEIVER_H
#define RF_RECEIVER_H

#include <Arduino.h>
#include "Config.h"
#include "EventQueue.h"
#include "SecurityTypes.h"

// Forward declaration
class EventQueue;

class RFReceiver {
public:
    RFReceiver();

    void begin();
    void update();
    void setEventQueue(EventQueue* queue) { eventQueuePtr = queue; }

    // Wake Flags (public for ISR access)
    volatile bool armFlag = false;
    volatile bool disarmFlag = false;
    volatile bool alarmFlag = false;

private:
    EventQueue* eventQueuePtr;
};

// Global RFReceiver instance for ISR access
extern RFReceiver* g_rfReceiver;

// ISR Handlers
void IRAM_ATTR onRFArm();
void IRAM_ATTR onRFDisarm();
void IRAM_ATTR onRFAlarm();

#endif // RF_RECEIVER_H
