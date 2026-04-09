#include "SecuritySystem.h"

// Global pointer for ISR access
SecuritySystem* g_securitySystem = nullptr;

// ISR Handlers - set wake flags (no queue access in ISR)
// TODO: Move this to a separate class like TouchController
void IRAM_ATTR onTouchWake() {
    if (g_securitySystem) {
        g_securitySystem->touchWakeFlag = true;
    }
}

SecuritySystem::SecuritySystem() {
    
    // Initialize config with defaults
    config.motionThreshold = MOTION_ACCEL_THRESHOLD;
    config.tiltThreshold = MOTION_TILT_THRESHOLD;
    config.warningTimeout = TIME_BETWEEN_WARNINGS;
    config.warningDebounce = WARNING_DEBOUNCE_TIME;
    config.alarmDuration = ALARM_TIMEOUT;
    config.rfEnabled = true;
    config.bluetoothEnabled = true;
    config.sleepEnabled = true;
    config.sensitivityLevel = 3;
    config.sleepTimeout = SLEEP_TIMEOUT;
    
    // Initialize status
    status.state = SecurityState::DISARMED;
    status.lastMotionWarningTime = 0;
    status.stateChangeTime = 0;
    status.lastActionTime = 0;
    status.relayState = true;  // Power connected by default
    status.bluetoothConnected = false;
    status.batteryPercent = 100;
    status.alarmTriggerCount = 0;
}

bool SecuritySystem::begin() {
    Serial.println("\n=== E-Bike Security System Starting ===");
    
    // Store global pointer for ISR access
    g_securitySystem = this;
    g_rfReceiver = &rfReceiver;
    g_motionSensor = &motionSensor;
    
    // Initialize components with feature toggles
    #if ENABLE_MOTION_SENSOR
    Serial.println("[INIT] Motion Sensor enabled");
    if (!motionSensor.begin()) {
        Serial.println("[ERROR] Motion sensor failed!");
        return false;
    }
    motionSensor.setEventQueue(&eventQueue);
    #else
    Serial.println("[SKIP] Motion Sensor disabled");
    #endif
    
    #if ENABLE_BUZZER
    Serial.println("[INIT] Buzzer enabled");
    buzzer.begin();
    buzzer.setEventQueue(&eventQueue);
    buzzer.setAlarmStatusPtr(&status.alarmActive);
    buzzer.playChirp(BuzzerPattern::SINGLE_BEEP);  // Startup chirp
    #else
    Serial.println("[SKIP] Buzzer disabled");
    #endif
    
    #if ENABLE_RELAY_CONTROL
    Serial.println("[INIT] Relay Control enabled");
    relay.begin();
    relay.setRelayStatePtr(&status.relayState);
    relay.enablePower();
    #else
    Serial.println("[SKIP] Relay Control disabled");
    #endif
    
    #if ENABLE_BLUETOOTH
    Serial.println("[INIT] Bluetooth enabled");
    if (!bluetooth.begin(&eventQueue)) {
        Serial.println("[ERROR] Bluetooth init failed, continuing...");
    }
    #else
    Serial.println("[SKIP] Bluetooth disabled");
    #endif
    
    #if ENABLE_RF_RECEIVER
    Serial.println("[INIT] RF Receiver enabled");
    rfReceiver.begin();
    rfReceiver.setEventQueue(&eventQueue);
    #else
    Serial.println("[SKIP] RF Receiver disabled");
    #endif
    
    #if ENABLE_TOUCH_WAKE
    attachInterrupt(digitalPinToInterrupt(PIN_TOUCH_WAKE), onTouchWake, RISING);
    Serial.println("[INIT] Touch interrupt attached to GPIO5");
    #endif
    
    #if ENABLE_LIGHT_SLEEP
    // Initialize SleepManager with dependencies and configure wake sources
    sleep.setEventQueue(&eventQueue);
    sleep.setLastActionTime(&status.lastActionTime);
    sleep.setSleepTimeout(&config.sleepTimeout);
    sleep.begin();
    Serial.println("[INIT] Light sleep configured");
    #endif
    
    
    // Initialize lastActionTime to current time (prevents immediate sleep on startup)
    status.lastActionTime = millis();
    
    status.state = SecurityState::DISARMED;
    Serial.println("[READY] System initialized - State: DISARMED\n");
    
    return true;
}

void SecuritySystem::update() {
    // Update buzzer patterns
    #if ENABLE_BUZZER
    buzzer.update();
    #endif
    
    // Update motion sensor (check interrupt flag and enqueue events)
    #if ENABLE_MOTION_SENSOR
    motionSensor.update();
    #endif
    
    // Process wake flags and post events
    #if ENABLE_TOUCH_WAKE
    if (touchWakeFlag) {
        touchWakeFlag = false;
        Event touchEvent(EventType::TOUCH_DETECTED, 0);
        eventQueue.enqueue(touchEvent);
        if (DEBUG_MODE) Serial.println("[EVENT] Touch detected!");
    }
    #endif
    
    // Process RF receiver button presses
    #if ENABLE_RF_RECEIVER
    rfReceiver.update();
    #endif
    
    // Process event queue
    Event event;
    while (eventQueue.dequeue(event)) {
        status.lastActionTime = millis();  // Track activity time for idle detection
        processEvent(event);
    }
    
    // Check state timeouts
    checkAlarmTimeout();
    
    // Send periodic BLE status
    #if ENABLE_BLUETOOTH
    if (config.bluetoothEnabled) {
        bluetooth.update(status);
    }
    #endif
    
    // Check idle timeout and enter sleep if conditions are met (only when ARMED)
    if (status.state == SecurityState::ARMED && !status.alarmActive) {
        sleep.update();
    }
}

void SecuritySystem::processEvent(const Event& event) {
    if (DEBUG_MODE) {
        Serial.print("Processing event: ");
        Serial.println((int)event.type);
    }
    
    switch (event.type) {
        case EventType::MOTION_DETECTED:
            handleMotionInput();
            break;
            
        case EventType::TOUCH_DETECTED:
            // Touch typically disarms the system
            if (status.state != SecurityState::DISARMED) {
                disarm();
            }
            break;
            
        case EventType::BLE_COMMAND: {
            CommandType cmd = (CommandType)event.value;
            switch (cmd) {
                case CommandType::ARM:
                    arm();
                    break;
                case CommandType::DISARM:
                    disarm();
                    break;
                case CommandType::STOP_ALARM:
                    stopAlarm();
                    break;
                case CommandType::GET_STATUS:
                    printStatus();
                    break;
                default:
                    break;
            }
            break;
        }
            
        case EventType::RF_COMMAND: {
            if (status.alarmActive) {
                stopAlarm();
            } else {
                CommandType cmd = (CommandType)event.value;
                switch (cmd) {
                    case CommandType::ARM:
                        arm();
                        break;
                    case CommandType::DISARM:
                        disarm();
                        break;
                    case CommandType::TRIGGER_ALARM:
                        playAlarm();
                        break;
                }
            }
            break;
        }
        default:
            Serial.println("[WARNING] Unhandled event type");
            break;
    }
}

void SecuritySystem::checkAlarmTimeout() {
    unsigned long now = millis();
    
    if (status.state == SecurityState::ALARM_TRIGGERED) {
        // ALARM timeout: return to ARMED state
        if (now - status.stateChangeTime >= config.alarmDuration) {
            Serial.println("ALARM timeout - returning to ARMED state");
            stopAlarm();
            relay.disablePower();
            changeState(SecurityState::ARMED);
        }
    }
}

void SecuritySystem::enterLightSleep() {
    if (DEBUG_MODE) {
        Serial.println("Entering light sleep...");
    }
    
    esp_light_sleep_start();
    
    if (DEBUG_MODE) {
        Serial.println("Woke from light sleep");
    }
}

void SecuritySystem::arm() {
    Serial.println("ARMING system...");

    relay.disablePower();
    buzzer.playChirpBlocking(BuzzerPattern::ARM_CONFIRM);
    if (status.state == SecurityState::DISARMED) {

        #if ENABLE_MOTION_SENSOR
        motionSensor.enableDetection();
        #endif
        changeState(SecurityState::ARMED);
        
        Serial.println("System ARMED - Monitoring active");
    }
    else {
        Serial.println("System already ARMED or in transition");
    }
}

void SecuritySystem::disarm() {
    if (status.state != SecurityState::DISARMED) {
        Serial.println("DISARMING system...");
        
        stopAlarm();
        relay.enablePower();
        
        #if ENABLE_MOTION_SENSOR
        motionSensor.disableDetection();
        #endif

        changeState(SecurityState::DISARMED);
        
        Serial.println("System DISARMED");
    }
    else {
        Serial.println("System already DISARMED");
    }
    buzzer.playChirp(BuzzerPattern::DISARM_CONFIRM);
}

void SecuritySystem::triggerAlarm() {
    if (status.state == SecurityState::ARMED) {
        
        Serial.println("THEFT DETECTED - FULL ALARM!");
        changeState(SecurityState::ALARM_TRIGGERED);
        playAlarm();
    }
}

void SecuritySystem::playAlarm() {
    buzzer.startAlarm();
    status.alarmActive = true;
    status.lastAlarmTriggerTime = millis();
    status.alarmTriggerCount++;
}

void SecuritySystem::stopAlarm() {
    Serial.println("Stopping alarm");
    buzzer.stopSound();
    status.alarmActive = false;

    status.lastActionTime = millis();
    
    if (status.state == SecurityState::ALARM_TRIGGERED) {
        changeState(SecurityState::ARMED);
    }
}

void SecuritySystem::changeState(SecurityState newState) {
    if (status.state != newState) {
        status.state = newState;
        status.stateChangeTime = millis();
        
        if (DEBUG_MODE) {
            printStatus();
        }
    }
}



void SecuritySystem::handleBluetoothInput() {
    if (!config.bluetoothEnabled) return;
    
}

// TODO: Add another check for reading the angle of vehicle (gyroscope)
void SecuritySystem::handleMotionInput() {
    float motionValue = motionSensor.getInterruptData();
    if (status.state == SecurityState::ARMED) {
        if (motionValue > config.motionThreshold) {
            Serial.println("Motion value over instant threshold");
            triggerAlarm();
        } else {
            unsigned long now = millis();
            if (now - status.lastMotionWarningTime > config.warningTimeout) {
                Serial.println("Minor motion detected");
                buzzer.playChirp(BuzzerPattern::WARNING);
                status.lastMotionWarningTime = now;
            } else if (now - status.lastMotionWarningTime > config.warningDebounce) {
                // If we get another motion event within the warning timeout but after the debounce time, trigger the alarm
                Serial.println("Repeated motion detected");
                triggerAlarm();
            }
        }
    }
}

void SecuritySystem::setConfig(const SecurityConfig& cfg) {
    config = cfg;
    // Save to EEPROM/Preferences here if needed
}

void SecuritySystem::printStatus() {
    Serial.println("\n--- System Status ---");
    Serial.print("State: ");
    switch (status.state) {
        case SecurityState::DISARMED: Serial.println("DISARMED"); break;
        case SecurityState::ARMING: Serial.println("ARMING"); break;
        case SecurityState::ARMED: Serial.println("ARMED"); break;
        case SecurityState::ALARM_TRIGGERED: Serial.println("ALARM TRIGGERED"); break;
        case SecurityState::ERROR: Serial.println("ERROR"); break;
    }
    Serial.print("Relay: ");
    Serial.println(status.relayState ? "ON" : "OFF");
    Serial.print("Bluetooth: ");
    Serial.println(status.bluetoothConnected ? "Connected" : "Disconnected");
    Serial.print("Alarm Count: ");
    Serial.println(status.alarmTriggerCount);
    Serial.println("-------------------\n");
}
