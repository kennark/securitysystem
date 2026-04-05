#ifndef SECURITY_TYPES_H
#define SECURITY_TYPES_H

#include <Arduino.h>

// ==================== ENUMERATIONS ====================

enum class SecurityState {
    DISARMED,           // System off, no monitoring
    ARMING,             // Grace period before armed
    ARMED,              // Monitoring active, motor disabled
    PRE_ALARM,          // Motion detected, warning period
    ALARM_TRIGGERED,    // Full alarm active
    ERROR               // System error state
};

enum class MotionEvent {
    NONE,
    BUMP,               // Small movement - warning threshold
    THEFT               // Strong movement - theft/stealing attempt
};

enum class BuzzerPattern {
    SINGLE_BEEP,
    DOUBLE_BEEP,
    ARM_CONFIRM,
    DISARM_CONFIRM,
    WARNING,
    ALARM,
    ERROR_BEEP
};

enum CommandType {
    ARM,
    DISARM,
    TRIGGER_ALARM,
    STOP_ALARM,
    GET_STATUS,
    SET_CONFIG,
    REBOOT
};

// ==================== STRUCTURES ====================

struct MotionData {
    float accelX, accelY, accelZ;       // m/s²
    float gyroX, gyroY, gyroZ;          // deg/s
    float temp;                          // °C
    float pitch, roll;                   // degrees
    unsigned long timestamp;             // milliseconds
};

struct SecurityConfig {
    // Motion Detection
    float motionThreshold;
    float tiltThreshold;
    uint32_t warningTimeout;
    uint32_t warningDebounce;
    
    // Alarm Settings
    uint32_t alarmDuration;
    
    // Features
    bool rfEnabled;
    bool bluetoothEnabled;
    bool deepSleepEnabled;
    
    // Sensitivity
    uint8_t sensitivityLevel;           // 1-5 (5=most sensitive)
};

struct SystemStatus {
    SecurityState state;
    MotionEvent lastEvent;
    unsigned long lastMotionWarningTime;
    unsigned long stateChangeTime;
    unsigned long lastAlarmTriggerTime;
    bool alarmActive;
    bool relayState;                    // true = power connected
    bool bluetoothConnected;
    uint8_t batteryPercent;
    uint16_t alarmTriggerCount;
};

struct Command {
    CommandType type;
    uint32_t value;                     // Optional parameter
};

#endif // SECURITY_TYPES_H
