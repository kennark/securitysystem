#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

#include <Arduino.h>
#include "SecurityTypes.h"
#include "Config.h"
#include "EventQueue.h"

// Forward declaration
class EventQueue;

// Pattern sequence structure
struct BuzzerStep {
    int frequency;          // Frequency in Hz (0 = silence)
};

class BuzzerController {
public:
    BuzzerController();
    
    void begin();
    void update();  // Call regularly to handle pattern timing
    void setEventQueue(EventQueue* queue) { eventQueuePtr = queue; }
    
    void startAlarm();
    void playChirp(BuzzerPattern pattern);
    void stopSound();
    
    bool isPlaying() const { return playing; }

private:
    EventQueue* eventQueuePtr;
    BuzzerPattern currentPattern;
    bool playing;
    bool alarmActive;
    unsigned long patternStartTime;
    int currentStepIndex;
    const BuzzerStep* currentSequence;
    int sequenceLength;
    
    // Helper methods
    void playSequence(const BuzzerStep* sequence, int length);
    void updateSequence();

    void updateAlarm();
};

#endif // BUZZER_CONTROLLER_H
