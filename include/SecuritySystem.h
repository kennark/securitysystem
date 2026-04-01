#ifndef SECURITY_SYSTEM_H
#define SECURITY_SYSTEM_H

#include <Arduino.h>
#include "SecurityTypes.h"
#include "Config.h"
#include "EventQueue.h"
#include "MotionSensor.h"
#include "RFReceiver.h"
#include "BuzzerController.h"
#include "BluetoothManager.h"
#include "RelayController.h"

class SecuritySystem {
public:
    SecuritySystem();
    
    bool begin();
    void update();
    
    // Event Queue Access
    EventQueue& getEventQueue() { return eventQueue; }
    
    // State Management
    void arm();
    void disarm();
    void triggerAlarm(MotionEvent event);
    void stopAlarm();
    
    // Status
    SecurityState getState() const { return status.state; }
    SystemStatus getStatus() const { return status; }
    
    // Configuration
    void setConfig(const SecurityConfig& cfg);
    SecurityConfig getConfig() const { return config; }
    
    // Wake Flags (public for ISR access)
    volatile bool motionWakeFlag = false;
    volatile bool touchWakeFlag = false;
    
private:
    // Event Queue
    EventQueue eventQueue;
    
    // Components
    MotionSensor motionSensor;
    RFReceiver rfReceiver;
    BuzzerController buzzer;
    BluetoothManager bluetooth;
    RelayController relay;
    
    // State
    SystemStatus status;
    SecurityConfig config;
    
    // Timers for state transitions
    unsigned long prealarmStartTime = 0;
    unsigned long alarmStartTime = 0;

    
    // State Machine
    void updateStateMachine();
    void changeState(SecurityState newState);
    void processEvent(const Event& event);
    void checkStateTimeouts();
    
    // Input Handlers
    void handleRFInput();
    void handleBluetoothInput();
    void handleMotionInput();
    
    // Wake Management
    void enterLightSleep();
    
    // Helper Methods
    void initializePins();
    void loadConfig();
    void saveConfig();
    void resetAlarmCounters();
    
    // Debug
    void printStatus();
};

// Global SecuritySystem instance for ISR access
extern SecuritySystem* g_securitySystem;

// ISR Handlers
void IRAM_ATTR onMotionWake();
void IRAM_ATTR onTouchWake();

#endif // SECURITY_SYSTEM_H
