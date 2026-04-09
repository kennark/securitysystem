#include "SleepManager.h"

SleepManager::SleepManager() {
}

void SleepManager::begin() {
    
    Serial.println("[INIT] SleepManager initialized");
    
    // Configure ESP32 wake sources for light sleep
    // MPU_INT_PIN: Motion sensor (MPU6500 interrupt pin)
    gpio_wakeup_enable((gpio_num_t)MPU_INT_PIN, GPIO_INTR_HIGH_LEVEL);
    Serial.print("[SLEEP] Wake source configured: GPIO");
    Serial.print(MPU_INT_PIN);
    Serial.println(" (Motion Sensor, rising edge)");

    // PIN_RF_RECEIVER_ALARM: RF receiver alarm pin
    gpio_wakeup_enable((gpio_num_t)PIN_RF_RECEIVER_ALARM, GPIO_INTR_HIGH_LEVEL);
    Serial.print("[SLEEP] Wake source configured: GPIO");
    Serial.print(PIN_RF_RECEIVER_ALARM);
    Serial.println(" (RF Receiver Alarm, rising edge)");

    // PIN_TOUCH_WAKE: Touch/Button pin for manual wake/disarm
    esp_sleep_enable_touchpad_wakeup();
    Serial.println("[SLEEP] Wake source configured: TOUCH");

    esp_sleep_enable_gpio_wakeup();
    
    Serial.print("[SLEEP] Sleep timeout set to ");
    Serial.print(*sleepTimeout);
    Serial.println("ms");
}

void SleepManager::update() {
    // Safety checks - do nothing if dependencies not set
    if (!eventQueue || !lastActionTime || !sleepTimeout || *sleepTimeout == 0) {
        return;
    }
    
    // Check if we can and should enter sleep
    if (canEnterSleep()) {
        enterSleep();
    }
}

bool SleepManager::canEnterSleep() const {
    // Prerequisite 1: Event queue must be empty
    if (eventQueue->size() > 0) {
        return false;
    }
    
    // Prerequisite 2: Idle timeout must be exceeded
    if (!isIdleTimeExceeded()) {
        return false;
    }
    
    return true;
}

bool SleepManager::isIdleTimeExceeded() const {
    unsigned long now = millis();
    unsigned long idleTime = now - *lastActionTime;
    
    // Account for millis() overflow (approx every 49 days)
    if (now < *lastActionTime) {
        // Overflow occurred, reset comparison
        idleTime = now;
    }
    
    return idleTime >= *sleepTimeout;
}

void SleepManager::enterSleep() {
    if (DEBUG_MODE) {
        unsigned long idleTime = millis() - *lastActionTime;
        Serial.print("[SLEEP] Entering light sleep after ");
        Serial.print(idleTime);
        Serial.println("ms idle");
        Serial.print("[SLEEP] System will wake on GPIO");
        Serial.print(MPU_INT_PIN);
        Serial.print(" (motion) or GPIO");
        Serial.print(PIN_TOUCH_WAKE);
        Serial.println(" (touch)");
    }
    
    // Enter ESP32 light sleep
    // RTC on, CPU paused, peripherals alive
    // Wake pins: MPU_INT_PIN (motion sensor) and PIN_TOUCH_WAKE (touch pin)
    esp_light_sleep_start();
    
    if (DEBUG_MODE) {
        Serial.println("[SLEEP] Woke from light sleep - processing wake event");
    }
}
