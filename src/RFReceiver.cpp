#include "RFReceiver.h"

// Global pointer for ISR access
RFReceiver* g_rfReceiver = nullptr;

// ISR Handlers - set wake flags (no queue access in ISR)
void IRAM_ATTR onRFArm() {
    if (g_rfReceiver) {
        g_rfReceiver->armFlag = true;
    }
}

void IRAM_ATTR onRFDisarm() {
    if (g_rfReceiver) {
        g_rfReceiver->disarmFlag = true;
    }
}

void IRAM_ATTR onRFAlarm() {
    if (g_rfReceiver) {
        g_rfReceiver->alarmFlag = true;
    }
}

RFReceiver::RFReceiver() {
}

void RFReceiver::begin() {
    pinMode(PIN_RF_RECEIVER_ON, INPUT_PULLDOWN);
    pinMode(PIN_RF_RECEIVER_OFF, INPUT_PULLDOWN);
    pinMode(PIN_RF_RECEIVER_ALARM, INPUT_PULLDOWN);
    
    attachInterrupt(digitalPinToInterrupt(PIN_RF_RECEIVER_ON), onRFArm, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_RF_RECEIVER_OFF), onRFDisarm, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_RF_RECEIVER_ALARM), onRFAlarm, RISING);
    
    Serial.println("[INIT] RF Receiver initialized - ARM on GPIO" + String(PIN_RF_RECEIVER_ON) + 
                   ", DISARM on GPIO" + String(PIN_RF_RECEIVER_OFF));
}

void RFReceiver::update() {
    if (!eventQueuePtr) return;
    
    // Check ARM flag
    if (armFlag) {
        armFlag = false;
        Event rfEvent(EventType::RF_COMMAND, (uint32_t)CommandType::ARM);
        eventQueuePtr->enqueue(rfEvent);
        if (DEBUG_MODE) Serial.println("[EVENT] RF ARM button pressed");
    }
    
    // Check DISARM flag
    if (disarmFlag) {
        disarmFlag = false;
        Event rfEvent(EventType::RF_COMMAND, (uint32_t)CommandType::DISARM);
        eventQueuePtr->enqueue(rfEvent);
        if (DEBUG_MODE) Serial.println("[EVENT] RF DISARM button pressed");
    }
    
    // Check ALARM flag
    if (alarmFlag) {
        alarmFlag = false;
        Event rfEvent(EventType::RF_COMMAND, (uint32_t)CommandType::TRIGGER_ALARM);
        eventQueuePtr->enqueue(rfEvent);
        if (DEBUG_MODE) Serial.println("[EVENT] RF ALARM button pressed");
    }
}