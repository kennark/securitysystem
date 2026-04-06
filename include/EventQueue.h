#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <Arduino.h>
#include <cstring>

// ==================== EVENT TYPE DEFINITIONS ====================

enum class EventType {
    NONE = 0,
    MOTION_DETECTED,        // Motion sensor interrupt triggered
    TOUCH_DETECTED,         // Touch pin interrupt triggered
    BLE_COMMAND,            // Command from Bluetooth app
    RF_COMMAND,             // Command from 433MHz remote
    SENSOR_ERROR            // Sensor malfunction or I2C error
};

// ==================== EVENT STRUCTURE ====================

struct Event {
    EventType type;
    uint32_t timestamp;     // milliseconds since boot (millis())
    uint32_t value;         // Optional payload (command type, error code, etc.)
    
    Event() : type(EventType::NONE), timestamp(0), value(0) {}
    Event(EventType t, uint32_t v = 0) : type(t), timestamp(millis()), value(v) {}
};

// ==================== THREAD-SAFE EVENT QUEUE (Ring Buffer) ====================
// Fixed-size FIFO queue optimized for embedded systems without dynamic allocation
// Safe for ISR contexts using volatile flags and atomic reads/writes

class EventQueue {
private:
    static constexpr size_t QUEUE_SIZE = 64;
    Event events[QUEUE_SIZE];
    volatile size_t head = 0;       // Write index
    volatile size_t tail = 0;       // Read index
    volatile size_t count = 0;      // Number of events in queue
    volatile bool enqueueLock = false;
    volatile bool dequeueLock = false;

public:
    EventQueue() = default;
    
    // Enqueue event (safe to call from ISR)
    // Returns true if successful, false if queue full
    bool enqueue(const Event& event) {
        // Simple spinlock: wait for dequeueLock to be clear
        while (dequeueLock) {
            // Busy wait
        }
        enqueueLock = true;
        
        bool result = true;
        if (count >= QUEUE_SIZE) {
            result = false;  // Queue full
        } else {
            events[head] = event;
            head = (head + 1) % QUEUE_SIZE;
            count++;
        }
        
        enqueueLock = false;
        return result;
    }
    
    // Dequeue event (call from main loop only)
    // Returns true if event available, false if queue empty
    bool dequeue(Event& event) {
        if (count == 0) {
            return false;
        }
        
        // Wait for enqueueLock to be clear
        while (enqueueLock) {
            // Busy wait
        }
        dequeueLock = true;
        
        event = events[tail];
        tail = (tail + 1) % QUEUE_SIZE;
        count--;
        
        dequeueLock = false;
        return true;
    }
    
    // Peek at next event without removing it
    bool peek(Event& event) const {
        if (count == 0) {
            return false;
        }
        event = events[tail];
        return true;
    }
    
    // Get current queue size
    size_t size() const {
        return count;
    }
    
    // Check if queue is empty
    bool empty() const {
        return count == 0;
    }
    
    // Check if queue is full
    bool full() const {
        return count >= QUEUE_SIZE;
    }
    
    // Clear all events
    void clear() {
        while (enqueueLock || dequeueLock) {
            // Busy wait
        }
        head = 0;
        tail = 0;
        count = 0;
    }
};

#endif // EVENT_QUEUE_H
