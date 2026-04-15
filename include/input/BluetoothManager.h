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
    
    bool begin();
    void update();
    
    void sendStatus();
    void sendAck(uint8_t commandCode);  // Immediate ACK feedback

    void setEventQueuePtr(EventQueue* ptr) { eventQueuePtr = ptr; }
    void setCurrentStatusPtr(SystemStatus* ptr) { currentStatus = ptr; }
    
private:
    EventQueue* eventQueuePtr;
    SystemStatus* currentStatus;
    bool statusSubscribed = false;
    unsigned long lastStatusSent = 0;  // Track last status update time
    
    NimBLEServer* pServer;
    NimBLEService* pService;
    NimBLECharacteristic* pCommandChar;
    NimBLECharacteristic* pStatusChar;

    void setStatusChar();
    
    // Server callbacks
    class ServerCallbacks : public NimBLEServerCallbacks {
        bool* deviceConnectedPtr;
    public:
        ServerCallbacks(bool* ptr) : deviceConnectedPtr(ptr) {}
        void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
        void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
    };
    
    // Command characteristic callbacks
    class CommandCallbacks : public NimBLECharacteristicCallbacks {
        BluetoothManager* manager;
    public:
        CommandCallbacks(BluetoothManager* mgr) : manager(mgr) {}
        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
    };


    class StatusCallbacks : public NimBLECharacteristicCallbacks {
        BluetoothManager* manager;
    public:
        StatusCallbacks(BluetoothManager* mgr) : manager(mgr) {}
        void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
        void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override;
    };
    
    friend class ServerCallbacks;
    friend class CommandCallbacks;
    friend class StatusCallbacks;
};

#endif // BLUETOOTH_MANAGER_H
