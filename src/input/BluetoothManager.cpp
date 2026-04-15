#include "input/BluetoothManager.h"

// ==================== BLUETOOTH MANAGER IMPLEMENTATION ====================

BluetoothManager::BluetoothManager() 
    : eventQueuePtr(nullptr), 
      pServer(nullptr),
      pService(nullptr),
      pCommandChar(nullptr),
      pStatusChar(nullptr) {
}

bool BluetoothManager::begin() {
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
    pServer->setCallbacks(new ServerCallbacks(&currentStatus->bluetoothConnected));
    
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

    pStatusChar->setCallbacks(new StatusCallbacks(this));
    
    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName(BLE_DEVICE_NAME);
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->addServiceUUID(BLE_COMMAND_UUID);
    pAdvertising->addServiceUUID(BLE_STATUS_UUID);
    pAdvertising->enableScanResponse(true);
    // pAdvertising->setPreferredParams(0x06, 0x12); // functions that help with iPhone compatibility
    pAdvertising->start();
    
    Serial.println("BLE initialized: " + String(BLE_DEVICE_NAME));
    
    return true;
}

void BluetoothManager::update() {
    if (!currentStatus || !currentStatus->bluetoothConnected) {
        return;
    }
    
    unsigned long now = millis();
    
    // Send full status every 5 seconds
    if (statusSubscribed && now - lastStatusSent >= STATUS_UPDATE_INTERVAL) {
        sendStatus();
        lastStatusSent = now;
    }
}

void BluetoothManager::sendStatus() {
    if (!pStatusChar || !currentStatus || !currentStatus->bluetoothConnected) {
        return;
    }
    
    setStatusChar();
    pStatusChar->notify();
    
    if (DEBUG_MODE) {
        Serial.println("BLE Status sent");
    }
}

void BluetoothManager::setStatusChar() {
    // Pack status: [state | alarm_count | relay_state | battery% | timestamp_high | timestamp_low]
    uint8_t statusData[6];
    statusData[0] = (uint8_t)currentStatus->state;
    statusData[1] = (uint8_t)(currentStatus->alarmTriggerCount & 0xFF);
    statusData[2] = currentStatus->relayState ? 1 : 0;
    statusData[3] = currentStatus->batteryPercent;
    // Add timestamp for phone to track freshness
    uint32_t ts = millis();
    statusData[4] = (ts >> 8) & 0xFF;
    statusData[5] = ts & 0xFF;
    
    pStatusChar->setValue(statusData, 6);
}

void BluetoothManager::sendAck(uint8_t commandCode) {
    if (!pStatusChar || !currentStatus || !currentStatus->bluetoothConnected) {
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

void BluetoothManager::ServerCallbacks::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    *deviceConnectedPtr = true;
    Serial.println("BLE Client connected");
}

void BluetoothManager::ServerCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    *deviceConnectedPtr = false;
    Serial.println("BLE Client disconnected");
    
    // Start advertising again
    NimBLEDevice::startAdvertising();
}

// ==================== COMMAND CHARACTERISTIC CALLBACKS ====================

void BluetoothManager::CommandCallbacks::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
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
    // manager->sendAck(commandByte);
    
    // Post command event to queue for processing
    if (manager->eventQueuePtr) {
        Event cmdEvent(EventType::BLE_COMMAND, (uint32_t)cmd);
        manager->eventQueuePtr->enqueue(cmdEvent);
    }
}

// ==================== STATUS CHARACTERISTIC CALLBACKS ====================

void BluetoothManager::StatusCallbacks::onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
    // Optionally handle read requests if needed
    Serial.println("BLE Status characteristic read");

    manager->setStatusChar();
}

/** Peer subscribed to notifications/indications */
void BluetoothManager::StatusCallbacks::onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str  = "Client ID: ";
    str             += connInfo.getConnHandle();
    str             += " Address: ";
    str             += connInfo.getAddress().toString();
    if (subValue == 0) {
        str += " Unsubscribed to ";
    } else if (subValue == 1) {
        str += " Subscribed to notifications for ";
    } else if (subValue == 2) {
        str += " Subscribed to indications for ";
    } else if (subValue == 3) {
        str += " Subscribed to notifications and indications for ";
    }
    str += std::string(pCharacteristic->getUUID());

    manager->statusSubscribed = (subValue == 1);

    Serial.printf("%s\n", str.c_str());
}
