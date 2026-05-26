

// =======  MPU6500 (I2C)  =======
#define PIN_SDA 6
#define PIN_SCL 7
#define MPU_INT_PIN 4

// 433MHz RF Receiver
#define PIN_RF_RECEIVER_ON 14
#define PIN_RF_RECEIVER_OFF 18
#define PIN_RF_RECEIVER_ALARM 9

// =======  Buzzer  =======
#define PIN_BUZZER 5

// =======  Relay  =======
#define PIN_RELAY 20

// Status LED (optional)
#define PIN_STATUS_LED 8



#define ENABLE_MOTION_SENSOR 1          // MPU6500 motion detection
#define ENABLE_BUZZER 1                 // Buzzer patterns
#define ENABLE_RELAY_CONTROL 1          // Relay power control
#define ENABLE_RF_RECEIVER 1            // RF receiver module
#define ENABLE_BLUETOOTH 1              // BLE communication
#define ENABLE_TOUCH_WAKE 0             // Touch pin wake functionality
#define ENABLE_LIGHT_SLEEP 0            // Light sleep
