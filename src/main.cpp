// Select modem
#include <ArduinoJson.h>
#include "definitions.h"
#include <Wire.h>
// presioner-alert/deviceID
#include <PubSubClient.h>

#define TINY_GSM_MODEM_SIM800 // Modem is SIM800L

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands
#define SerialAT Serial1
#define low_bat 25

JsonDocument doc;
String mqttPayload;
#include <TinyGsmClient.h>
#include <TinyGPS++.h>
TinyGsm modem(SerialAT);

#define deviceID 1542
TwoWire I2CPower = TwoWire(0);
// Setting up the GPS
static const uint32_t GPSBaud = 9600;
unsigned long lastPublishTime = 0L;
unsigned long currentTime = 0L;
TinyGPSPlus gps;
float latitude;
float longitude;

TinyGsmClient client(modem);
PubSubClient mqtt(client);

unsigned long lastReconnectAttempt = 0L;

bool setPowerBoostKeepOn(int en)
{
  I2CPower.beginTransmission(IP5306_ADDR);
  I2CPower.write(IP5306_REG_SYS_CTL0);
  if (en)
  {
    I2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  }
  else
  {
    I2CPower.write(0x35); // 0x37 is default reg value
  }
  return I2CPower.endTransmission() == 0;
}

void checkLocation(String message)
{
  if (message.indexOf("outside") >= 0)
  {
    SerialMon.println("OUTSIDE AREA");
    ledcWrite(1, 200);
    digitalWrite(BUZZER, HIGH);
  }
  else
  {

    SerialMon.println("INSIDE AREA");
    ledcWrite(1, 0);
    digitalWrite(BUZZER, LOW);
  }
}

void mqttCallback(char *topic, byte *message, unsigned int len)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < len; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println("Received message : " + messageTemp);
  checkLocation(messageTemp);
  Serial.println();
}

boolean mqttConnect()
{
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker without username and password
  boolean status = mqtt.connect("GsmClientN");

  // Or, if you want to authenticate MQTT:
  // boolean status = mqtt.connect("GsmClientN", mqttUsername, mqttPassword);

  if (status == false)
  {
    SerialMon.println(" fail");
    ESP.restart();
    return false;
  }
  SerialMon.println(" success");

  if (mqtt.subscribe(SubTopic))
    SerialMon.println("Successfully subscribed ");
  else
    SerialMon.println("Error subscribed ");

  return mqtt.connected();
}
int getBatteryLevel()
{
  int rawValue = analogRead(35);
  float voltageLevel = (rawValue / 4095.0) * 2 * 1.1 * 3.3; // calculate voltage level
  float batteryFraction = voltageLevel / MAX_BATTERY_VOLTAGE;

  // Serial.println((String) "Raw:" + rawValue + " Voltage:" + voltageLevel + "V Percent: " + (batteryFraction * 100) + "%");
  return floor(batteryFraction * 100);
}

void sendInfo()
{
  currentTime = millis();
  if ((currentTime - lastPublishTime >= 5000))
  {

    lastPublishTime = currentTime;
    if (gps.location.isValid() && gps.location.isUpdated())
    {
      ledcWrite(3, 250);
      latitude = (gps.location.lat()); // Storing the Lat. and Lon.
      longitude = (gps.location.lng());

      Serial.print("LAT:  ");
      Serial.println(latitude, 6); // float to x decimal places
      Serial.print("LONG: ");
      Serial.println(longitude, 6);

      doc["latitude"] = latitude;
      doc["longitude"] = longitude;
      SerialMon.println("Successfully get location  ...");
    }
    else
    {

      ledcWrite(3, 0);
      doc["latitude"] = -1;
      doc["longitude"] = -1;
      SerialMon.println("Data didn't change ...");
    }
    doc["deviceID"] = deviceID;
    doc["battery"] = getBatteryLevel();

    serializeJson(doc, mqttPayload);
    mqtt.publish(PublishTopic, mqttPayload.c_str());
    SerialMon.println("Publishded on " + String(PublishTopic));
  }
}
void checkModuleStatus(bool mqttStatus, int Battery)
{
  if (mqttStatus)
    ledcWrite(2, 250);
  else
    ledcWrite(2, 80);
  if (Battery <= low_bat)
  {
    ledcWrite(0, 200);
    digitalWrite(BUZZER, HIGH);
    delay(250);
    digitalWrite(BUZZER, LOW);
    delay(250);
    digitalWrite(BUZZER, HIGH);
    delay(250);
    digitalWrite(BUZZER, LOW);
  }
  else
  {
    ledcWrite(0, 0);
  }
}

void handleGPS()
{

  byte c;
  while (Serial.available() > 0)
  {
    c = Serial.read();
    if (gps.encode(c))
    {

      sendInfo();
    }
  }
  // sendGPSNotConnected();
}
void setUpPins()
{
  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // set LEDs and buzzer pins
  ledcSetup(0, frequency, resolution);
  ledcSetup(1, frequency, resolution);
  ledcSetup(2, frequency, resolution);
  ledcSetup(3, frequency, resolution);

  ledcAttachPin(LED_RED, 0);
  ledcAttachPin(LED_YELLOW, 1);
  ledcAttachPin(LED_BLUE, 2);
  ledcAttachPin(LED_GREEN, 3);

  pinMode(BUZZER, OUTPUT);
}
void setup()
{
  // Keep power when running from battery
  // Set console baud rate
  SerialMon.begin(GPSBaud);
  delay(10);

  // Start I2C communication
  I2CPower.begin(I2C_SDA, I2C_SCL, 400000);

  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  SerialMon.println("Wait...");

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(6000);

  setUpPins();

  ledcWrite(2, 80);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3)
  {
    modem.simUnlock(GSM_PIN);
  }

  SerialMon.print("Connecting to APN: ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    SerialMon.println(" fail");
    ESP.restart();
  }
  else
  {
    SerialMon.println(" OK");
  }

  if (modem.isGprsConnected())
  {
    SerialMon.println("GPRS connected");
  }

  // MQTT Broker setup
  SerialMon.println("Connecing to MQTT");
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
}

void loop()
{

  if (!mqtt.connected())
  {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    unsigned long t = millis();
    Serial.println(t - lastReconnectAttempt > 10000L);
    if (t - lastReconnectAttempt > 10000L)
    {
      lastReconnectAttempt = t;
      if (mqttConnect())
      {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }
  mqtt.loop();
  checkModuleStatus(mqtt.connected(), getBatteryLevel());
  handleGPS();
  mqtt.loop();
}
