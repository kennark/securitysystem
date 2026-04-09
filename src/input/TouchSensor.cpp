#include "input/TouchSensor.h"

TouchSensor* g_touchSensor = nullptr;

void IRAM_ATTR onTouchWake() {
    if (g_touchSensor) {
        g_touchSensor->touchWakeFlag = true;
    }
}

TouchSensor::TouchSensor() {
}

void TouchSensor::begin() {
    touchAttachInterrupt(PIN_TOUCH_WAKE, onTouchWake, 20000);

}

void TouchSensor::update() {
    if (!eventQueuePtr) return;

    if (touchWakeFlag) {
        touchWakeFlag = false;
        Event touchEvent(EventType::TOUCH_DETECTED);
        eventQueuePtr->enqueue(touchEvent);
        if (DEBUG_MODE) Serial.println("[EVENT] Touch detected");
    }
}