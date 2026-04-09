#ifndef SECURITY_TYPES_H
#define SECURITY_TYPES_H

#include <Arduino.h>

// ==================== ENUMERATIONS ====================

enum class SecurityState {
    DISARMED,           // System off, no monitoring
    ARMING,             // Grace period before armed
    ARMED,              // Monitoring active, motor disabled
    ALARM_TRIGGERED,    // Full alarm active
    ERROR               // System error state
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
    float accel;                        // m/s² across all axes combined
    float gyro;                         // degrees of deviation from initial calibration
    unsigned long timestamp;            // milliseconds
};

struct SecurityConfig {
    // Motion Detection
    float motionThreshold;
    float tiltThreshold;
    
    // Alarm Settings
    uint32_t warningTimeout;
    uint32_t warningDebounce;
    uint32_t alarmDuration;
    
    // Features
    bool rfEnabled;
    bool bluetoothEnabled;
    bool sleepEnabled;

    // Sleep settings
    uint32_t sleepTimeout; // 0 = off
    
    // Sensitivity
    uint8_t sensitivityLevel;           // 1-5 (5=most sensitive)
};

struct SystemStatus {
    SecurityState state;
    unsigned long stateChangeTime;
    unsigned long lastMotionWarningTime;
    unsigned long lastAlarmTriggerTime;
    unsigned long lastActionTime;
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
