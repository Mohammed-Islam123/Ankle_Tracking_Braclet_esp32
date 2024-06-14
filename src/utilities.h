#include <ArduinoJson.h>
#include "definitions.h"

#include <Wire.h>
#include <TinyGsmClient.h>
#include <TinyGPS++.h>
#include <PubSubClient.h>
bool setPowerBoostKeepOn(int en)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
    {
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    }
    else
    {
        Wire.write(0x35); // 0x37 is default reg value
    }
    return Wire.endTransmission() == 0;
}

void checkLocation(String message)
{
    if (message.indexOf("outside") > 0)
    {
        SerialMon.println("OUTSIDE AREA");
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(BUZZER, HIGH);
    }
    else
    {

        SerialMon.println("INSIDE AREA");
        digitalWrite(LED_YELLOW, LOW);
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

bool mqttConnect()
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
    mqtt.subscribe(SubTopic);

    return mqtt.connected();
}
int getBatteryLevel()
{
    int rawValue = analogRead(35);
    float voltageLevel = (rawValue / 4095.0) * 2 * 1.1 * 3.3; // calculate voltage level
    float batteryFraction = voltageLevel / MAX_BATTERY_VOLTAGE;

    Serial.println((String) "Raw:" + rawValue + " Voltage:" + voltageLevel + "V Percent: " + (batteryFraction * 100) + "%");
    return batteryFraction * 100;
}
void sendInfo()
{
    unsigned long currentTime = millis(); // Get the current time
    if (currentTime - lastPublishTime >= 5000)
    {
        lastPublishTime = currentTime;
        if (gps.location.isValid() && gps.location.isUpdated())
        {

            latitude = (gps.location.lat()); // Storing the Lat. and Lon.
            longitude = (gps.location.lng());

            satellites = gps.satellites.value(); // get number of satellites
            Serial.print("LAT:  ");
            Serial.println(latitude, 6); // float to x decimal places
            Serial.print("LONG: ");
            Serial.println(longitude, 6);
            Serial.println("Satalleits : " + String(satellites, 3));

            doc["latitude"] = latitude;
            doc["longitude"] = longitude;
        }
        else
        {
            doc["latitude"] = -1;
            doc["longitude"] = -1;

            SerialMon.println("Data didn't change ...");
            // mqtt.publish(PublishTopic, "Bye MQTT");
        }
        doc["deviceID"] = deviceID;
        doc["battery"] = getBatteryLevel();

        serializeJson(doc, mqttPayload);
        mqtt.publish(PublishTopic, mqttPayload.c_str());
        getBatteryLevel();
    }
}
void checkModuleStatus(bool mqttStatus, int Battery)
{
    if (mqttStatus)
        digitalWrite(LED_WHITE, HIGH);
    else
        digitalWrite(LED_WHITE, LOW);
    if (Battery <= low_bat)
    {
        digitalWrite(BUZZER, HIGH);
        delay(250);
        digitalWrite(BUZZER, LOW);
        delay(250);
        digitalWrite(BUZZER, HIGH);
        delay(250);
        digitalWrite(BUZZER, LOW);
        digitalWrite(LED_RED, HIGH);
    }
    else
    {
        digitalWrite(LED_RED, LOW);
    }
    if (Battery <= very_low_bat)
    {
        delay(250);
        digitalWrite(BUZZER, HIGH);
        delay(250);
        digitalWrite(BUZZER, LOW);
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
            Serial.println(char(c));
            sendInfo();
        }
    }
}
