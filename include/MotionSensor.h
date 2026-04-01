#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>
#include <MPU6500_WE.h>
#include <Wire.h>
#include "SecurityTypes.h"
#include "Config.h"

class MotionSensor {
public:
    MotionSensor(int sda = PIN_SDA, int scl = PIN_SCL);

    bool begin();
    bool calibrate();
    
    void enableDetection();
    void disableDetection();
    
    void printData();

    float getTemperature();

    float getInterruptData();

private:
    MPU6500_WE mpu = MPU6500_WE(MPU_ADDR);
    
};

#endif // MOTION_SENSOR_H
