# TBD - AI boilerplate currently
This repo will be used to hold code related to a micromobility security system.
Updates will be pushed by module


## E-Bike/Scooter Security System

A comprehensive security system for electric bikes and scooters using ESP32-S3 with multiple protection features.

## Hardware Components

- **ESP32-S3** - Main controller with Bluetooth
- **MPU6050** - Motion/tilt detection sensor
- **433MHz RF Receiver** - Remote control via key fob
- **Active Buzzer (3-24V)** - Alarm sound
- **Relay Module** - Motor power control

## Features

### Current Implementation

- ✅ **Motion Detection** - MPU6050 accelerometer and gyroscope monitoring
- ✅ **Tilt Detection** - Detects when bike is moved or tilted
- ✅ **Remote Control** - 433MHz RF key fob for arm/disarm
- ✅ **Bluetooth Control** - Smartphone app integration
- ✅ **Progressive Alarm** - Warning period before full alarm
- ✅ **Power Cutoff** - Relay disconnects motor when armed
- ✅ **Multiple Alarm Patterns** - Different sounds for different events
- ✅ **Configurable Sensitivity** - Adjust motion thresholds
- ✅ **Status Notifications** - Real-time updates via Bluetooth

### Planned Enhancements

- 🔲 GPS tracking module integration
- 🔲 Geofencing (auto-arm when walking away)
- 🔲 Battery monitoring
- 🔲 Deep sleep mode for power saving
- 🔲 Configuration storage (EEPROM/Preferences)
- 🔲 Multiple user profiles
- 🔲 Alarm history logging

## Wiring Diagram

```
ESP32-S3 Pin Connections:
├── I2C (MPU6050)
│   ├── GPIO21 (SDA) → MPU6050 SDA
│   └── GPIO22 (SCL) → MPU6050 SCL
├── GPIO18 → 433MHz RF Receiver DATA
├── GPIO19 → Buzzer (+)
├── GPIO23 → Relay IN
└── GPIO2  → Status LED (optional)

Power:
├── ESP32: 5V via USB or regulated supply
├── MPU6050: 3.3V from ESP32
├── Buzzer: 12V/24V from bike battery
├── Relay: 5V (VCC), controls main motor power
└── RF Receiver: 5V
```

## Pin Configuration

Edit `include/Config.h` to match your wiring:

```cpp
#define PIN_SDA 21              // I2C Data
#define PIN_SCL 22              // I2C Clock
#define PIN_RF_RECEIVER 18      // RF receiver data pin
#define PIN_BUZZER 19           // Buzzer control
#define PIN_RELAY 23            // Relay control
```

## RF Remote Setup

1. Upload the code to your ESP32
2. Open Serial Monitor (115200 baud)
3. Press buttons on your 433MHz remote
4. Note the codes displayed in the serial monitor
5. Update `Config.h` with your codes:

```cpp
#define RF_CODE_ARM      5393    // Your button 1 code
#define RF_CODE_DISARM   5396    // Your button 2 code
#define RF_CODE_PANIC    5400    // Your button 3 code
```

## Bluetooth Protocol

The system uses BLE (Bluetooth Low Energy) for smartphone communication.

### Service UUID
`4fafc201-1fb5-459e-8fcc-c5c9c331914b`

### Characteristics

**Command Characteristic** (Write)
- UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Format: `COMMAND:VALUE`
- Examples:
  - `ARM:0` - Arm the system
  - `DISARM:0` - Disarm the system
  - `CONFIG:3` - Set sensitivity level (1-5)
  - `TRIGGER:0` - Trigger panic alarm
  - `STOP:0` - Stop alarm
  - `STATUS:0` - Request status update
  - `REBOOT:0` - Restart ESP32

**Status Characteristic** (Read/Notify)
- UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a9`
- Format: JSON
- Example: `{"state":"ARMED","event":"NONE","relay":0,"battery":100,"count":0}`

## Usage

### Serial Commands

Type these commands in the Serial Monitor:

- `ARM` - Arm the security system
- `DISARM` - Disarm the security system
- `STATUS` - Display current status
- `HELP` - Show available commands

### Operation Modes

1. **DISARMED** - Normal operation, motor power connected
2. **ARMED** - Monitoring active, motor power disconnected
3. **PRE-ALARM** - Motion detected, warning beeps (10s grace period)
4. **ALARM** - Full alarm activated, loud continuous sound

### Arming Sequence

1. Park your e-bike
2. Press ARM button on RF remote (or via smartphone)
3. System beeps once to confirm
4. Motion sensor calibrates (takes 1-2 seconds)
5. Motor power disconnects
6. System is now monitoring for motion

### Disarming Sequence

1. Press DISARM button on RF remote (or via smartphone)
2. System beeps twice to confirm
3. Alarm stops (if triggered)
4. Motor power reconnects
5. Ready to ride

## Smartphone App Development

To create a companion app, use the BLE protocol above. Here's a quick start:

### React Native / Expo Example

```javascript
import { BleManager } from 'react-native-ble-plx';

const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const COMMAND_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const STATUS_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';

// Send ARM command
await device.writeCharacteristicWithResponseForService(
  SERVICE_UUID,
  COMMAND_UUID,
  btoa('ARM:0')
);

// Listen for status updates
device.monitorCharacteristicForService(
  SERVICE_UUID,
  STATUS_UUID,
  (error, characteristic) => {
    if (characteristic?.value) {
      const status = JSON.parse(atob(characteristic.value));
      console.log('Status:', status);
    }
  }
);
```

### Flutter Example

```dart
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

final serviceUuid = Guid('4fafc201-1fb5-459e-8fcc-c5c9c331914b');
final commandUuid = Guid('beb5483e-36e1-4688-b7f5-ea07361b26a8');

// Send DISARM command
await characteristic.write(utf8.encode('DISARM:0'));
```

## Customization

### Adjust Motion Sensitivity

In `Config.h`:

```cpp
#define MOTION_ACCEL_THRESHOLD 2.0f   // Lower = more sensitive
#define MOTION_TILT_THRESHOLD 15.0f   // Degrees of tilt
```

Or use Bluetooth: `CONFIG:1` (most sensitive) to `CONFIG:5` (least sensitive)

### Change Alarm Timing

```cpp
#define PRE_ALARM_DELAY 10000      // Warning period (ms)
#define ALARM_TIMEOUT 120000       // Max alarm duration (ms)
```

### Customize Buzzer Sounds

Edit `src/BuzzerController.cpp` to modify alarm patterns.

## Troubleshooting

### MPU6050 Not Found
- Check I2C wiring (SDA/SCL)
- Try different I2C pins
- Verify MPU6050 has 3.3V power

### RF Remote Not Working
- Check wiring to GPIO18
- Verify 5V power to receiver
- Run code to capture your remote codes
- Update codes in Config.h

### Bluetooth Not Connecting
- Check device name in BLE scanner
- Ensure ESP32-S3 Bluetooth is enabled
- Try restarting the ESP32
- Check distance (BLE range ~10m)

### False Alarms
- Increase sensitivity level: `CONFIG:4` or `CONFIG:5`
- Adjust thresholds in Config.h
- Recalibrate on level surface

## Build and Upload

```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Open serial monitor
pio device monitor
```

## License

MIT License - Feel free to modify and use for your projects

## Contributing

Contributions welcome! Areas for improvement:
- GPS tracking integration
- Web dashboard
- Mobile app (React Native/Flutter)
- Power optimization
- Additional sensor support

---

**Stay Safe! Secure Your Ride! 🚴🔒**
