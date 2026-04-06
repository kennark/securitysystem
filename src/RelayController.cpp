#include "RelayController.h"

RelayController::RelayController()
    : relayStatePtr(nullptr) {
}

void RelayController::begin() {
    pinMode(PIN_RELAY, OUTPUT_OPEN_DRAIN);
    disablePower();
    Serial.println("Relay initialized on pin " + String(PIN_RELAY) + " (power disabled)");
}
// Relay (and its light) is active on LOW
void RelayController::enablePower() {
    if (relayStatePtr && !*relayStatePtr) {
        digitalWrite(PIN_RELAY, LOW);
        *relayStatePtr = true;
        Serial.println("Power ENABLED via relay");
    }
}

void RelayController::disablePower() {
    if (relayStatePtr && *relayStatePtr) {
        digitalWrite(PIN_RELAY, HIGH);
        *relayStatePtr = false;
        Serial.println("Power DISABLED via relay");
    }
}