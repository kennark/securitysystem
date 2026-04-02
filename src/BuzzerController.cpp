#include "BuzzerController.h"

// ==================== PATTERN SEQUENCES ====================
// Each pattern is defined as an array of frequency steps
// Duration is fixed per note (BUZZER_NOTE_SPEED) for simplicity
// Frequency of 0 means silence

const BuzzerStep PATTERN_SINGLE_BEEP[] = {
    BUZZER_FREQ_BEEP,
};

const BuzzerStep PATTERN_DOUBLE_BEEP[] = {
    BUZZER_FREQ_BEEP,
    0,  // Silence
    BUZZER_FREQ_BEEP,
};

const BuzzerStep PATTERN_WARNING[] = {
    BUZZER_FREQ_WARNING,
    0,  // Silence
    BUZZER_FREQ_WARNING,
};

const BuzzerStep PATTERN_ARM_CONFIRM[] = {
    BUZZER_FREQ_ARM,
    BUZZER_FREQ_ARM + 200,
};

const BuzzerStep PATTERN_DISARM_CONFIRM[] = {
    BUZZER_FREQ_DISARM,
    BUZZER_FREQ_DISARM - 200,
};

const BuzzerStep PATTERN_ERROR_BEEP[] = {
    BUZZER_FREQ_WARNING,
    0,  // Silence
    BUZZER_FREQ_WARNING,
};

// ==================== CONSTRUCTOR & INITIALIZATION ====================

BuzzerController::BuzzerController() 
    : eventQueuePtr(nullptr),
      currentPattern(BuzzerPattern::SINGLE_BEEP),
      playing(false),
      alarmActive(false),
      patternStartTime(0),
      currentStepIndex(0),
      currentSequence(nullptr),
      sequenceLength(0) {
}

void BuzzerController::begin() {
    // Configure ledC PWM for the buzzer
    ledcAttachPin(PIN_BUZZER, BUZZER_PWM_CHANNEL);
    
    // Initialize buzzer as off
    ledcWriteTone(BUZZER_PWM_CHANNEL, 0);
    
    Serial.println("Buzzer initialized on pin " + String(PIN_BUZZER) + 
                   " (PWM channel " + String(BUZZER_PWM_CHANNEL) + ")");
}

// ==================== PUBLIC METHODS ====================

void BuzzerController::startAlarm() {
    if (!playing) {
        playing = true;
        alarmActive = true;
        patternStartTime = millis();
        updateAlarm();
        Serial.println("Alarm tone started");
    }
}

void BuzzerController::playChirp(BuzzerPattern pattern) {
    if (!playing) {
        currentPattern = pattern;
        
        // Select appropriate sequence based on pattern
        const BuzzerStep* sequence = nullptr;
        int length = 0;
        
        switch (pattern) {
            case BuzzerPattern::SINGLE_BEEP:
                sequence = PATTERN_SINGLE_BEEP;
                length = sizeof(PATTERN_SINGLE_BEEP) / sizeof(PATTERN_SINGLE_BEEP[0]);
                break;
            case BuzzerPattern::DOUBLE_BEEP:
                sequence = PATTERN_DOUBLE_BEEP;
                length = sizeof(PATTERN_DOUBLE_BEEP) / sizeof(PATTERN_DOUBLE_BEEP[0]);
                break;
            case BuzzerPattern::ARM_CONFIRM:
                sequence = PATTERN_ARM_CONFIRM;
                length = sizeof(PATTERN_ARM_CONFIRM) / sizeof(PATTERN_ARM_CONFIRM[0]);
                break;
            case BuzzerPattern::DISARM_CONFIRM:
                sequence = PATTERN_DISARM_CONFIRM;
                length = sizeof(PATTERN_DISARM_CONFIRM) / sizeof(PATTERN_DISARM_CONFIRM[0]);
                break;
            case BuzzerPattern::WARNING:
                sequence = PATTERN_WARNING;
                length = sizeof(PATTERN_WARNING) / sizeof(PATTERN_WARNING[0]);
                break;
            case BuzzerPattern::ERROR_BEEP:
                sequence = PATTERN_ERROR_BEEP;
                length = sizeof(PATTERN_ERROR_BEEP) / sizeof(PATTERN_ERROR_BEEP[0]);
                break;
            default:
                return;
        }
        
        playSequence(sequence, length);
    }
}

void BuzzerController::update() {
    if (!playing) {
        return;
    }
    if (alarmActive) {
        updateAlarm();
    } else {
        updateSequence();
    }
}

void BuzzerController::stopSound() {
    if (playing) {
        alarmActive = false;
        playing = false;
        ledcWriteTone(BUZZER_PWM_CHANNEL, 0);
        Serial.println("Sound stopped");
        
        // Post BUZZER_DONE event
        if (!alarmActive && eventQueuePtr) {
            Event doneEvent(EventType::BUZZER_DONE);
            eventQueuePtr->enqueue(doneEvent);
        }
    }
}

// ==================== PRIVATE HELPER METHODS ====================

void BuzzerController::playSequence(const BuzzerStep* sequence, int length) {
    if (sequence == nullptr || length <= 0) {
        playing = false;
        ledcWriteTone(BUZZER_PWM_CHANNEL, 0);
        return;
    }
    
    currentSequence = sequence;
    sequenceLength = length;
    patternStartTime = millis();
    currentStepIndex = 0;
    playing = true;
    
    // Set initial frequency
    ledcWriteTone(BUZZER_PWM_CHANNEL, currentSequence[0].frequency);
    //ledcWriteNote(BUZZER_PWM_CHANNEL, NOTE_G, 6);
}

void BuzzerController::updateSequence() {
    if (currentSequence == nullptr || sequenceLength == 0) {
        playing = false;
        ledcWriteTone(BUZZER_PWM_CHANNEL, 0);
        return;
    }

    unsigned long now = millis();
    unsigned long elapsed = now - patternStartTime;
    
    int currentStep = elapsed / BUZZER_NOTE_SPEED;

    if (currentStep >= sequenceLength) { // Chirp length exceeded, stop sound
        stopSound();
        return;
    }
    else if (currentStep != currentStepIndex) {
        // Step changed, update frequency
        currentStepIndex = currentStep;
        ledcWriteTone(BUZZER_PWM_CHANNEL, currentSequence[currentStepIndex].frequency);
    }

    //Serial.println("Buzzer update - step: " + String(currentStepIndex) + 
    //              ", freq: " + String(currentSequence[currentStepIndex].frequency) + "length" + String(sequenceLength));
}

void BuzzerController::updateAlarm() {
    if (alarmActive) {
        unsigned long now = millis();
        unsigned long elapsed = now - patternStartTime;

        if (elapsed >= ALARM_TIMEOUT) {
            stopSound();
            Serial.println("Alarm timeout reached, stopping alarm tone");
            return;
        }

        int currentStep = elapsed / BUZZER_NOTE_SPEED;
        if (currentStep % 2 == 0) {
            ledcWriteTone(BUZZER_PWM_CHANNEL, BUZZER_FREQ_ALARM);
        } else {
            ledcWriteTone(BUZZER_PWM_CHANNEL, BUZZER_FREQ_ALARM2);
        }
    }
}
