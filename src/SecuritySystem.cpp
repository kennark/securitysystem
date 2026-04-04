#include "SecuritySystem.h"

// Global pointer for ISR access
SecuritySystem* g_securitySystem = nullptr;

// ISR Handlers - set wake flags (no queue access in ISR)
void IRAM_ATTR onMotionWake() {
    if (g_securitySystem) {
        g_securitySystem->motionWakeFlag = true;
    }
}

void IRAM_ATTR onTouchWake() {
    if (g_securitySystem) {
        g_securitySystem->touchWakeFlag = true;
    }
}

SecuritySystem::SecuritySystem() {
    
    // Initialize config with defaults
    config.motionThreshold = MOTION_BUMP_THRESHOLD;
    config.tiltThreshold = MOTION_THEFT_THRESHOLD;  // Using tiltThreshold field for theft threshold
    config.preAlarmDelay = PRE_ALARM_DELAY;
    config.alarmDuration = ALARM_TIMEOUT;
    config.rfEnabled = true;
    config.bluetoothEnabled = true;
    config.deepSleepEnabled = false;
    config.sensitivityLevel = 3;
    
    // Initialize status
    status.state = SecurityState::DISARMED;
    status.lastEvent = MotionEvent::NONE;
    status.lastMotionTime = 0;
    status.stateChangeTime = 0;
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
    
    // Initialize components with feature toggles
    #if ENABLE_MOTION_SENSOR
    Serial.println("[INIT] Motion Sensor enabled");
    if (!motionSensor.begin()) {
        Serial.println("[ERROR] Motion sensor failed!");
        return false;
    }
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
    
    #if ENABLE_MOTION_SENSOR
    attachInterrupt(digitalPinToInterrupt(MPU_INT_PIN), onMotionWake, RISING);
    Serial.println("[INIT] Motion interrupt attached to GPIO4");
    #endif
    
    #if ENABLE_TOUCH_WAKE
    attachInterrupt(digitalPinToInterrupt(PIN_TOUCH_WAKE), onTouchWake, RISING);
    Serial.println("[INIT] Touch interrupt attached to GPIO5");
    #endif
    
    #if ENABLE_LIGHT_SLEEP
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);    // Motion sensor on GPIO4
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_5, 1);    // Touch pin on GPIO5
    Serial.println("[INIT] Light sleep wake sources configured");
    #endif
    
    status.state = SecurityState::DISARMED;
    Serial.println("[READY] System initialized - State: DISARMED\n");
    
    return true;
}

void SecuritySystem::update() {
    // Update buzzer patterns
    #if ENABLE_BUZZER
    buzzer.update();
    #endif
    
    // Process wake flags and post events
    #if ENABLE_MOTION_SENSOR
    if (motionWakeFlag) {
        motionWakeFlag = false;
        Event motionEvent(EventType::MOTION_DETECTED, (uint32_t)MotionEvent::THEFT);
        eventQueue.enqueue(motionEvent);
        if (DEBUG_MODE) Serial.println("[EVENT] Motion detected!");
    }
    #endif
    
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
        processEvent(event);
    }
    
    // Check state timeouts
    checkStateTimeouts();
    
    // Send periodic BLE status
    #if ENABLE_BLUETOOTH
    if (config.bluetoothEnabled) {
        bluetooth.update(status);
    }
    #endif
    
    // Enter light sleep if idle and ARMED
    #if ENABLE_LIGHT_SLEEP
    if (status.state == SecurityState::ARMED && eventQueue.empty()) {
        enterLightSleep();
    }
    #endif
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
            CommandType cmd = (CommandType)event.value;
            switch (cmd) {
                case CommandType::ARM:
                    if (status.alarmActive) {
                        stopAlarm();
                    }
                    arm();
                    break;
                case CommandType::DISARM:
                    if (status.alarmActive) { 
                        stopAlarm();
                    }
                    disarm();
                    break;
                case CommandType::TRIGGER_ALARM:
                    if (!status.alarmActive) {
                        playAlarm();
                    } else {
                        stopAlarm();
                    }
                    break;
            }
            break;
        }
            
        case EventType::TIMER_EXPIRED:
            checkStateTimeouts();
            break;
            
        case EventType::BUZZER_DONE:
            // Buzzer pattern finished, continue state machine
            break;
            
        default:
            break;
    }
}

void SecuritySystem::checkStateTimeouts() {
    unsigned long now = millis();
    
    if (status.state == SecurityState::PRE_ALARM) {
        // PRE_ALARM timeout: transition to ALARM_TRIGGERED
        if (now - status.stateChangeTime >= config.preAlarmDelay) {
            Serial.println("PRE_ALARM timeout - escalating to FULL ALARM");
            changeState(SecurityState::ALARM_TRIGGERED);
            buzzer.startAlarm();
            relay.disablePower();
            status.alarmTriggerCount++;
        }
    } else if (status.state == SecurityState::ALARM_TRIGGERED) {
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
    if (status.state == SecurityState::DISARMED) {
        Serial.println("ARMING system...");
        
        relay.disablePower();

        #if ENABLE_MOTION_SENSOR
        motionSensor.enableDetection();
        #endif
        
        changeState(SecurityState::ARMED);
        
        Serial.println("System ARMED - Monitoring active");
    }
    else {
        Serial.println("System already ARMED or in transition");
    }
    buzzer.playChirp(BuzzerPattern::ARM_CONFIRM);
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
    
    if (status.state == SecurityState::ALARM_TRIGGERED || 
        status.state == SecurityState::PRE_ALARM) {
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

void SecuritySystem::handleMotionInput() {
    Serial.println("Motion detected - reading sensor data...");
    float motionValue = motionSensor.getInterruptData();
    Serial.print("Motion value (g): ");
    Serial.println(motionValue);
    status.lastMotionTime = millis();
    if (status.state == SecurityState::ARMED) {
        // Get latest motion data from sensor

        // TODO: For small acceleration values, play a warning sound and possibly give a timer. 
        // If another bump is detected during that time, sound the alarm
        // Maybe add another check for reading the angle of vehicle (gyroscope)
        
        // For now, trigger THEFT alarm on any motion interrupt
        triggerAlarm();
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
        case SecurityState::PRE_ALARM: Serial.println("PRE-ALARM"); break;
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
