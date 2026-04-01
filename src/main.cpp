#include <Arduino.h>
#include "SecuritySystem.h"
#include "Config.h"

// Global security system instance
SecuritySystem securitySystem;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting E-Bike Security System...");
    // Initialize the security system
    if (!securitySystem.begin()) {
        Serial.println("CRITICAL ERROR: System initialization failed!");
        while (1) {
            delay(1000);
        }
    }
}

void loop() {
    // Main system update loop
    securitySystem.update();
    
    // Optional: Serial commands for testing
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toUpperCase();
        
        if (command == "ARM") {
            securitySystem.arm();
        } else if (command == "DISARM") {
            securitySystem.disarm();
        } else if (command == "STATUS") {
            SystemStatus status = securitySystem.getStatus();
            Serial.println("\n=== System Status ===");
            Serial.print("State: ");
            Serial.println((int)status.state);
            Serial.print("Relay: ");
            Serial.println(status.relayState ? "ON" : "OFF");
            Serial.print("BT Connected: ");
            Serial.println(status.bluetoothConnected ? "YES" : "NO");
            Serial.print("Alarm Count: ");
            Serial.println(status.alarmTriggerCount);
            Serial.println("====================\n");
        } else if (command == "HELP") {
            Serial.println("\n=== Available Commands ===");
            Serial.println("ARM     - Arm the security system");
            Serial.println("DISARM  - Disarm the security system");
            Serial.println("STATUS  - Show system status");
            Serial.println("HELP    - Show this help");
            Serial.println("=========================\n");
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(10);
}