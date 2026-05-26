

// =======  MPU6500 (I2C)  =======
#define PIN_SDA 8
#define PIN_SCL 9
#define MPU_INT_PIN 18

// 433MHz RF Receiver
#define PIN_RF_RECEIVER_ON 7
#define PIN_RF_RECEIVER_OFF 6
#define PIN_RF_RECEIVER_ALARM 5

// =======  Touch Wake  =======
#define PIN_TOUCH_WAKE 1    // Capacitive touch pin for wake, translates to GPIO1

// =======  Buzzer  =======
#define PIN_BUZZER 4

// =======  Relay  =======
#define PIN_RELAY 12

// Status LED (optional)
#define PIN_STATUS_LED 2



#define ENABLE_MOTION_SENSOR 1          // MPU6500 motion detection
#define ENABLE_BUZZER 1                 // Buzzer patterns
#define ENABLE_RELAY_CONTROL 1          // Relay power control
#define ENABLE_RF_RECEIVER 1            // RF receiver module
#define ENABLE_BLUETOOTH 1              // BLE communication
#define ENABLE_TOUCH_WAKE 1             // Touch pin wake functionality
#define ENABLE_LIGHT_SLEEP 1            // Light sleep
