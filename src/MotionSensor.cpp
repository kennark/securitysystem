#include "MotionSensor.h"

MotionSensor::MotionSensor(int sda, int scl) {
    Wire.begin(sda, scl);
}

bool MotionSensor::begin() {
    if (!mpu.init()) {
        Serial.println("Failed to find MPU6500 chip");
        return false;
    }
    
    Serial.println("MPU6500 Found!");

    mpu.setSampleRateDivider(5);
    mpu.setAccRange(MPU6500_ACC_RANGE_8G);
    mpu.enableAccDLPF(true);
    mpu.setAccDLPF(MPU6500_DLPF_6);
    mpu.setIntPinPolarity(MPU6500_ACT_HIGH);
    mpu.enableIntLatch(true);
    mpu.enableClearIntByAnyRead(false);

    mpu.enableInterrupt(MPU6500_WOM_INT);

    mpu.setWakeOnMotionThreshold(MPU_WAKE_THRESHOLD);  // 128 = ~0.5 g

    delay(100);
    return calibrate();
}

bool MotionSensor::calibrate() {
    Serial.println("Calibrating MPU6500...");
    
    mpu.autoOffsets();

    Serial.println("Done!");
    return true;
}


void MotionSensor::enableDetection() {

    mpu.sleep(false);
    delay(100);
    calibrate();
    mpu.enableWakeOnMotion(MPU6500_WOM_ENABLE, MPU6500_WOM_COMP_DISABLE);
        
}

void MotionSensor::disableDetection() {
    mpu.enableWakeOnMotion(MPU6500_WOM_DISABLE, MPU6500_WOM_COMP_DISABLE);

    mpu.sleep(true);
    delay(100);
}

float MotionSensor::getInterruptData() {
    uint8_t intStatus = mpu.readAndClearInterrupts();
    if (mpu.checkInterrupt(intStatus, MPU6500_WOM_INT)) {
        Serial.println("Wake-on-Motion Interrupt Detected!");
    }

    return mpu.getResultantG(mpu.getGValues());
}

float MotionSensor::getTemperature() {
    return mpu.getTemperature();
}