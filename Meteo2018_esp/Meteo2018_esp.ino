#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>

float temperatureInside;
float temperatureOutside;
float temperatureWater;
String currentTime;
String currentDate;
float manualModeSetPoint;
float nightSetPoint;
float daySetPoint;
bool heaterStatus;
int modeNumber;
int loopCycleTime;

char auth[] = "64eb1e89df674887b797183a7d3150a5";
char ssid[] = "7SkyHome";
char pass[] = "89191532537";
BlynkTimer timer;

BLYNK_WRITE(V0)
{
  manualModeSetPoint = param.asFloat();
  StaticJsonBuffer<400> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["manSetPoint"] = manualModeSetPoint;
  root.printTo(Serial);
  Serial.println();
}

BLYNK_WRITE(V2)
{
  heaterStatus = param.asInt();
  StaticJsonBuffer<400> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["heatSt"] = heaterStatus;
  root.printTo(Serial);
  Serial.println();
}

BLYNK_WRITE(V5)
{
  modeNumber = param.asInt();
  StaticJsonBuffer<400> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["modeNumber"] = modeNumber;
  root.printTo(Serial);
  Serial.println();
}

BLYNK_WRITE(V6)
{
  daySetPoint = param.asFloat();
  StaticJsonBuffer<400> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["daySetPoint"] = daySetPoint;
  root.printTo(Serial);
  Serial.println();
}

BLYNK_WRITE(V7)
{
  nightSetPoint = param.asFloat();
  StaticJsonBuffer<400> jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["nightSetPoint"] = nightSetPoint;
  root.printTo(Serial);
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendDataToBlynkServer);
}

void loop()
{
  int loopCycleBegin = millis();
  receiveJsonDataBySerial();
  Blynk.run();
  timer.run();
  loopCycleTime = millis() - loopCycleBegin;
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
  Blynk.virtualWrite(V8, temperatureOutside);
  Blynk.virtualWrite(V9, temperatureWater);
  Blynk.virtualWrite(V10, loopCycleTime);
}

void receiveJsonDataBySerial()
{ 
  if (Serial.available() > 0)
  {
    StaticJsonBuffer<650> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(Serial);
    temperatureInside = root["tempIn"];
    temperatureOutside = root["tempOut"];
    temperatureWater = root["tempW"];
    const char* _currentDate = root["date"];
    const char* _currentTime = root["time"];
    currentDate = String(_currentDate);
    currentTime = String(_currentTime);
    manualModeSetPoint = root["manSetPoint"]; 
    heaterStatus = root["heatSt"];
    modeNumber = root["modeNumber"];
    daySetPoint = root["daySetPoint"];
    nightSetPoint = root["nightSetPoint"];
  }
}
