#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>
#include <MPU6500_WE.h>
#include <Wire.h>
#include "SecurityTypes.h"
#include "Config.h"
#include "EventQueue.h"

// Forward declaration
class EventQueue;

class MotionSensor {
public:
    MotionSensor(int sda = PIN_SDA, int scl = PIN_SCL);

    bool begin();
    bool calibrate();
    
    void enableDetection();
    void disableDetection();

    float getTemperature();

    float getInterruptData();
    
    void update();
    void setEventQueue(EventQueue* queue) { eventQueuePtr = queue; }
    
    // Wake Flag (public for ISR access)
    volatile bool motionWakeFlag = false;

private:
    MPU6500_WE mpu = MPU6500_WE(MPU_ADDR);
    EventQueue* eventQueuePtr = nullptr;
};

// Global MotionSensor instance for ISR access
extern MotionSensor* g_motionSensor;

// ISR Handler
void IRAM_ATTR onMotionWake();

#endif // MOTION_SENSOR_H
