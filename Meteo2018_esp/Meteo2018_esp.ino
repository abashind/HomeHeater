#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>

float temperatureInside;
float temperatureOutside;
const char* currentTime;
const char* currentDate;
float setPointCelsius;
bool heaterStatus;
int screenNumber;

char auth[] = "64eb1e89df674887b797183a7d3150a5";
char ssid[] = "7SkyHome";
char pass[] = "89191532537";
BlynkTimer timer;

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
}

void loop()
{
  receiveJsonData();
  Blynk.run();
  timer.run();
}

void myTimerEvent()
{
  Blynk.virtualWrite(V0, setPointCelsius);
  Blynk.virtualWrite(V1, currentTime);
  Blynk.virtualWrite(V2, heaterStatus);
  Blynk.virtualWrite(V3, temperatureInside);
  Blynk.virtualWrite(V4, currentDate);
}

void receiveJsonData()
{ 
  if (Serial.available())
  {
    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject& root = jsonBuffer.parseObject(Serial);
    temperatureInside = root["tempIn"];
    temperatureOutside = root["tempOut"];
    currentDate = root["date"];
    currentTime = root["time"];
    setPointCelsius = root["setPoint"]; 
    heaterStatus = root["heatSt"];
    screenNumber = root["screen"];
  }
}
