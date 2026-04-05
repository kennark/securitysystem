#include "MotionSensor.h"

// Global pointer for ISR access
MotionSensor* g_motionSensor = nullptr;

// ISR Handler - set wake flag (no queue access in ISR)
void IRAM_ATTR onMotionWake() {
    if (g_motionSensor) {
        g_motionSensor->motionWakeFlag = true;
    }
}

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

    delay(50);
    
    attachInterrupt(digitalPinToInterrupt(MPU_INT_PIN), onMotionWake, RISING);
    Serial.println("[INIT] Motion interrupt attached to GPIO" + String(MPU_INT_PIN));
    
    return true;
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

void MotionSensor::update() {
    if (!eventQueuePtr) return;

    // Check motion wake flag
    if (motionWakeFlag) {
        motionWakeFlag = false;
        Event motionEvent(EventType::MOTION_DETECTED, (uint32_t)MotionEvent::THEFT);
        eventQueuePtr->enqueue(motionEvent);
        if (DEBUG_MODE) Serial.println("[EVENT] Motion detected!");
    }
}