## Micromobility Security System

A comprehensive security system **prototype** for use with (electric) bikes and scooters.

## Current Hardware Components

- **ESP32-S3** - Main controller with Bluetooth
- **MPU6500** - Motion/tilt detection sensor
- **433MHz RF Receiver** - Remote control via key fob with up to 4 buttons
- **Active Buzzer (3-24V)** - Alarm sound
- **Relay Module** - Motor power control

In future, the components are planned to match real needs (after a working prototype)

## Features

### Current Implementation

- ✅ **Motion Detection** - MPU6500 accelerometer monitoring
- ✅ **Remote Control** - 433MHz RF key fob for arm/disarm
- ✅ **Progressive Alarm** - Warning period before full alarm
- ✅ **Power Cutoff** - Relay disconnects motor when armed (aimed to replace/complement the key system)
- ✅ **Multiple Alarm Patterns** - Different sounds for different events
- ✅ **Configurable Sensitivity** - Adjust motion thresholds

### Planned Enhancements

- 🔲 Battery operation with optional connection to ebike batteries
- 🔲 **Bluetooth control** - Will be integrated with a companion app (currently there exists some AI code) 
- 🔲 Tilt detection
- 🔲 GPS tracking module integration
- 🔲 Geofencing (auto-arm when walking away)
- 🔲 Battery monitoring
- 🔲 Deep sleep mode for power saving
- 🔲 Configuration storage (EEPROM/Preferences)
- 🔲 Multiple user profiles
- 🔲 Alarm history logging

## Pin Configuration

Edit `include/Config.h` to match your wiring:

```cpp
#define PIN_SDA 21              // I2C Data
#define PIN_SCL 22              // I2C Clock
#define PIN_BUZZER 4           // Buzzer control
#define PIN_RELAY 12            // Relay control
```

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

1. Park your ride
2. Press ARM button on RF remote
3. System beeps to confirm
4. Motor power disconnects
5. Motion sensor calibrates (takes 1-2 seconds)
7. System is now monitoring for motion

### Disarming Sequence

1. Press DISARM button on RF remote (or via smartphone)
2. System beeps twice to confirm
3. Alarm stops (if triggered)
4. Motor power reconnects
5. Ready to ride

## Customization

### Adjust Motion Sensitivity

In `Config.h`:

```cpp
#define MOTION_ACCEL_THRESHOLD 2.0f   // Lower = more sensitive
#define MOTION_TILT_THRESHOLD 15.0f   // Degrees of tilt
```

### Change Alarm Timing

```cpp
#define PRE_ALARM_DELAY 10000      // Warning period (ms)
#define ALARM_TIMEOUT 120000       // Max alarm duration (ms)
```

### Customize Buzzer Sounds

Edit `src/BuzzerController.cpp` to modify alarm patterns.

## Troubleshooting

### MPU6500 Not Found
- Check I2C wiring (SDA/SCL)
- Try different I2C pins
- Verify MPU6500 has 3.3V power
- Possibly wrong model MPU

### RF Remote Not Working
- Check wiring
- Verify 3.3V power to receiver
- Update pin numbers in Config.h

### False Alarms
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
