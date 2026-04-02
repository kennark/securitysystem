#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <esp_sleep.h>

// =======  MPU6500 (I2C)  =======
#define PIN_SDA 8
#define PIN_SCL 9
#define MPU_ADDR 0x68
#define MPU_INT_PIN 18
// Choose 1 (= 4 mg) ..... 255 (= 1020 mg); 
#define MPU_WAKE_THRESHOLD 128  // ~0.5 g

// 433MHz RF Receiver
#define PIN_RF_RECEIVER_ON 7
#define PIN_RF_RECEIVER_OFF 6
#define PIN_RF_RECEIVER_ALARM 5

// =======  Touch Wake  =======
#define PIN_TOUCH_WAKE 5    // Capacitive touch or button pin for wake/disarm

// =======  Buzzer  =======
#define PIN_BUZZER 4
#define BUZZER_PWM_CHANNEL 0          // ledC channel for buzzer

#define BUZZER_NOTE_SPEED 250         // ms per note in patterns

// =======  Relay  =======
#define PIN_RELAY 12

// Status LED (optional)
#define PIN_STATUS_LED 2

// ==================== SECURITY CONFIGURATION ====================
// Motion Detection Thresholds (m/s²)
#define MOTION_BUMP_THRESHOLD 1.0f       // Low threshold - small bumps (warning)
#define MOTION_THEFT_THRESHOLD 2.5f      // High threshold - actual theft attempt (full alarm)
#define MOTION_CHECK_INTERVAL 100        // Check motion every 100ms

// Alarm Timing (milliseconds)
#define PRE_ALARM_DELAY 5000UL           // 5 seconds warning period for bumps
#define ALARM_TIMEOUT 10000UL           // 2 minutes max alarm duration

// Buzzer Patterns
#define BUZZER_FREQ_BEEP 1100            // Hz - generic beep
#define BUZZER_FREQ_WARNING 1000         // Hz - warning beep
#define BUZZER_FREQ_ARM 1500             // Hz - arm confirmation
#define BUZZER_FREQ_DISARM 1200          // Hz - disarm confirmation
#define BUZZER_FREQ_ALARM 2000           // Hz - full alarm
#define BUZZER_FREQ_ALARM2 2500          // Hz - alternate alarm tone

// Bluetooth Configuration
#define BLE_DEVICE_NAME "EBike_Security"
#define BLE_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // These should be auto generated on first boot
#define BLE_COMMAND_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_STATUS_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a9"

// RF Remote Codes 

// Power Management
// Note: System uses light sleep when ARMED with BLE enabled
// Motion sensor (GPIO4) and touch pin (GPIO5) wake the device from sleep

// ==================== FEATURE TOGGLES FOR TESTING ====================
#define ENABLE_MOTION_SENSOR 0          // Test MPU6500 motion detection
#define ENABLE_BUZZER 1                 // Test buzzer patterns
#define ENABLE_RELAY_CONTROL 1          // Test relay power control
#define ENABLE_RF_RECEIVER 1            // Disabled until RF module ready
#define ENABLE_BLUETOOTH 0              // Test BLE communication
#define ENABLE_TOUCH_WAKE 0             // Test touch pin wake functionality
#define ENABLE_LIGHT_SLEEP 0            // Test sleep/wake cycles

// Debug
#define DEBUG_MODE true                  // Enable serial debugging

#endif // CONFIG_H