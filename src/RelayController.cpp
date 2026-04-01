#include "RelayController.h"

RelayController::RelayController(int pin)
    : relayPin(pin), powered(false) {
}

void RelayController::begin() {
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);  // LOW = power disabled
    powered = false;
    Serial.println("Relay initialized on pin " + String(relayPin) + " (power disabled)");
}

void RelayController::enablePower() {
    if (!powered) {
        digitalWrite(relayPin, HIGH);
        powered = true;
        Serial.println("Power ENABLED via relay");
    }
}

void RelayController::disablePower() {
    if (powered) {
        digitalWrite(relayPin, LOW);
        powered = false;
        Serial.println("Power DISABLED via relay");
    }
}