#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "SecurityTypes.h"
#include "Config.h"
#include "EventQueue.h"

// Forward declarations
class SecuritySystem;

class BluetoothManager {
public:
    BluetoothManager();
    
    bool begin(EventQueue* eventQueue);
    void update(const SystemStatus& status);  // Periodic status updates
    bool isConnected() const { return deviceConnected; }
    
    // Send status updates to connected client
    void sendStatus(const SystemStatus& status);
    void sendAck(uint8_t commandCode);  // Immediate ACK feedback
    
private:
    EventQueue* eventQueuePtr;
    bool deviceConnected = false;
    unsigned long lastStatusSent = 0;  // Track last status update time
    static constexpr unsigned long STATUS_UPDATE_INTERVAL = 5000;  // Send status every 5 seconds
    
    NimBLEServer* pServer;
    NimBLEService* pService;
    NimBLECharacteristic* pCommandChar;
    NimBLECharacteristic* pStatusChar;
    
    // Server callbacks
    class ServerCallbacks : public NimBLEServerCallbacks {
        BluetoothManager* manager;
    public:
        ServerCallbacks(BluetoothManager* mgr) : manager(mgr) {}
        void onConnect(NimBLEServer* pServer) override;
        void onDisconnect(NimBLEServer* pServer) override;
    };
    
    // Command characteristic callbacks
    class CommandCallbacks : public NimBLECharacteristicCallbacks {
        BluetoothManager* manager;
    public:
        CommandCallbacks(BluetoothManager* mgr) : manager(mgr) {}
        void onWrite(NimBLECharacteristic* pCharacteristic) override;
    };
    
    friend class ServerCallbacks;
    friend class CommandCallbacks;
};

#endif // BLUETOOTH_MANAGER_H
