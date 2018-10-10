#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>

float temperatureInside;
float temperatureOutside;
const char* currentTime;
const char* currentDate;
float manualModeSetPoint;
float nightSetPoint;
float daySetPoint;
bool heaterStatus;
int modeNumber;

char auth[] = "64eb1e89df674887b797183a7d3150a5";
char ssid[] = "7SkyHome";
char pass[] = "89191532537";
BlynkTimer timer;

BLYNK_WRITE(V0)
{
  manualModeSetPoint = param.asFloat();
}

BLYNK_WRITE(V5)
{
  modeNumber = param.asInt();
}

BLYNK_WRITE(V6)
{
  daySetPoint = param.asFloat();
}

BLYNK_WRITE(V7)
{
  nightSetPoint = param.asFloat();
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendDataToBlynkServer);
  timer.setInterval(500L, sendJsonDataToSerial);
}

void loop()
{
  receiveJsonDataFromSerial();
  sendJsonDataToSerial();
  Blynk.run();
  timer.run();
}

void sendDataToBlynkServer()
{
  Blynk.virtualWrite(V0, manualModeSetPoint);
  Blynk.virtualWrite(V1, currentTime);
  Blynk.virtualWrite(V2, heaterStatus);
  Blynk.virtualWrite(V3, temperatureInside);
  Blynk.virtualWrite(V4, currentDate);
  Blynk.virtualWrite(V5, modeNumber);
  Blynk.virtualWrite(V6, daySetPoint);
  Blynk.virtualWrite(V7, nightSetPoint);
}

void receiveJsonDataFromSerial()
{ 
  if (Serial.available())
  {
    StaticJsonBuffer<450> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(Serial);
    temperatureInside = root["tempIn"];
    temperatureOutside = root["tempOut"];
    currentDate = root["date"];
    currentTime = root["time"];
    manualModeSetPoint = root["manSetPoint"]; 
    heaterStatus = root["heatSt"];
    modeNumber = root["modeNumber"];
    daySetPoint = root["daySetPoint"];
    nightSetPoint = root["nightSetPoint"];
  }
}

void sendJsonDataToSerial()
{ 
  if (Serial.available())
  {
    StaticJsonBuffer<450> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["manSetPoint"] = manualModeSetPoint;
    root["daySetPoint"] = daySetPoint;
    root["nightSetPoint"] = nightSetPoint;
    root["modeNumber"] = nightSetPoint;
    root.printTo(Serial);
  }
}


