#include "RelayController.h"

RelayController::RelayController()
    : powered(false) {
}

void RelayController::begin() {
    pinMode(PIN_RELAY, OUTPUT);
    disablePower();
    Serial.println("Relay initialized on pin " + String(PIN_RELAY) + " (power disabled)");
}
// Relay (and its light) is active on LOW
void RelayController::enablePower() {
    if (!powered) {
        digitalWrite(PIN_RELAY, LOW);
        powered = true;
        Serial.println("Power ENABLED via relay");
    }
}

void RelayController::disablePower() {
    if (powered) {
        digitalWrite(PIN_RELAY, HIGH);
        powered = false;
        Serial.println("Power DISABLED via relay");
    }
}