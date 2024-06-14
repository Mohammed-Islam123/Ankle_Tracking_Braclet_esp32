#define TINY_GSM_MODEM_SIM800 // Modem is SIM800L

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands
#define SerialAT Serial1
#define low_bat 25

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define GsmBaud 115200
#define GSM_PIN ""
// TTGO T-Call pins
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define I2C_SDA 21
#define I2C_SCL 22
// LEDs and buzzer pins
#define LED_RED 18    // LOW BATTERY => channel 0
#define LED_YELLOW 25 // OUTSIDE AREA channel => 1
#define LED_BLUE 2    // power on / CONNECTED to broker => channel 2
#define LED_GREEN 18  // Connected to GPS satalleites(getting location) =>channel 3
#define BUZZER 19
#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00
const int MAX_ANALOG_VAL = 4095;
const float MAX_BATTERY_VOLTAGE = 4.2;
const short frequency = 5000;
const short resolution = 8;
const char *SubTopic = "prisioner-alert/1542";

// Your GPRS credentials (leave empty, if not needed)
const char apn[] = "internet"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = "";    // GPRS User
const char gprsPass[] = "";    // GPRS Password
// SIM card PIN (leave empty, if not defined)
const char simPIN[] = "";

const short deviceID = 1542;
// MQTT details
const char *broker = "test.mosquitto.org"; // Public IP address or domain name
const char *mqttUsername = "";             // MQTT username
const char *mqttPassword = "";             // MQTT password

const char *PublishTopic = "zmalaPublish";
