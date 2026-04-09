#include "input/BluetoothManager.h"

// ==================== BLUETOOTH MANAGER IMPLEMENTATION ====================

BluetoothManager::BluetoothManager() 
    : eventQueuePtr(nullptr), 
      pServer(nullptr),
      pService(nullptr),
      pCommandChar(nullptr),
      pStatusChar(nullptr) {
}

bool BluetoothManager::begin(EventQueue* eventQueue) {
    eventQueuePtr = eventQueue;
    
    if (!eventQueuePtr) {
        Serial.println("ERROR: EventQueue pointer is null!");
        return false;
    }
    
    // Initialize NimBLE device
    NimBLEDevice::init(BLE_DEVICE_NAME);
    
    // Create BLE server
    pServer = NimBLEDevice::createServer();
    if (!pServer) {
        Serial.println("ERROR: Failed to create BLE server");
        return false;
    }
    
    // Set server callbacks
    pServer->setCallbacks(new ServerCallbacks(this));
    
    // Create BLE service
    pService = pServer->createService(BLE_SERVICE_UUID);
    if (!pService) {
        Serial.println("ERROR: Failed to create BLE service");
        return false;
    }
    
    // Create command characteristic (write only - phone sends commands)
    pCommandChar = pService->createCharacteristic(
        BLE_COMMAND_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    pCommandChar->setCallbacks(new CommandCallbacks(this));
    
    // Create status characteristic (read/notify - device sends status)
    pStatusChar = pService->createCharacteristic(
        BLE_STATUS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    
    // Start the service
    pService->start();
    
    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone compatibility
    pAdvertising->setMaxPreferred(0x12);
    NimBLEDevice::startAdvertising();
    
    Serial.println("BLE initialized: " + String(BLE_DEVICE_NAME));
    
    return true;
}

void BluetoothManager::update(const SystemStatus& status) {
    if (!deviceConnected) {
        return;
    }
    
    unsigned long now = millis();
    
    // Send full status every 5 seconds
    if (now - lastStatusSent >= STATUS_UPDATE_INTERVAL) {
        sendStatus(status);
        lastStatusSent = now;
    }
}

void BluetoothManager::sendStatus(const SystemStatus& status) {
    if (!pStatusChar || !deviceConnected) {
        return;
    }
    
    // Pack status: [state | alarm_count | relay_state | battery% | timestamp_high | timestamp_low]
    uint8_t statusData[6];
    statusData[0] = (uint8_t)status.state;
    statusData[1] = (uint8_t)(status.alarmTriggerCount & 0xFF);
    statusData[2] = status.relayState ? 1 : 0;
    statusData[3] = status.batteryPercent;
    // Add timestamp for phone to track freshness
    uint32_t ts = millis();
    statusData[4] = (ts >> 8) & 0xFF;
    statusData[5] = ts & 0xFF;
    
    pStatusChar->setValue(statusData, 6);
    pStatusChar->notify();
    
    if (DEBUG_MODE) {
        Serial.println("BLE Status sent");
    }
}

void BluetoothManager::sendAck(uint8_t commandCode) {
    if (!pStatusChar || !deviceConnected) {
        return;
    }
    
    // Send ACK: [0xFF = ACK flag, command code]
    uint8_t ackData[2] = {0xFF, commandCode};
    pStatusChar->setValue(ackData, 2);
    pStatusChar->notify();
    
    if (DEBUG_MODE) {
        Serial.print("BLE ACK sent for command: ");
        Serial.println((int)commandCode);
    }
}

// ==================== SERVER CALLBACKS ====================

void BluetoothManager::ServerCallbacks::onConnect(NimBLEServer* pServer) {
    manager->deviceConnected = true;
    Serial.println("BLE Client connected");
    
    // Post connection event to queue
    if (manager->eventQueuePtr) {
        Event connEvent(EventType::BLE_COMMAND, (uint32_t)CommandType::GET_STATUS);
        manager->eventQueuePtr->enqueue(connEvent);
    }
}

void BluetoothManager::ServerCallbacks::onDisconnect(NimBLEServer* pServer) {
    manager->deviceConnected = false;
    Serial.println("BLE Client disconnected");
    
    // Start advertising again
    NimBLEDevice::startAdvertising();
}

// ==================== COMMAND CHARACTERISTIC CALLBACKS ====================

void BluetoothManager::CommandCallbacks::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    
    if (rxValue.length() == 0) {
        return;
    }
    
    uint8_t commandByte = rxValue[0];
    CommandType cmd = (CommandType)commandByte;
    
    if (DEBUG_MODE) {
        Serial.print("BLE Command received: ");
        Serial.println((int)cmd);
    }
    
    // IMMEDIATE ACK - fast feedback to phone
    manager->sendAck(commandByte);
    
    // Post command event to queue for processing
    if (manager->eventQueuePtr) {
        Event cmdEvent(EventType::BLE_COMMAND, (uint32_t)cmd);
        manager->eventQueuePtr->enqueue(cmdEvent);
    }
}
