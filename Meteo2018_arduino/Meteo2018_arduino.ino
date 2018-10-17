#include <GyverEncoder.h>
#include <Arduino.h>
#include <U8x8lib.h>
#include <Wire.h>
#include <DS3231.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>
#include <ArduinoJson.h>
#include "OneButton.h"

#define TEMP_OUTSIDE 2
#define TEMP_INSIDE 3
#define UP_PIN 4
#define DOWN_PIN 5
#define TEMP_WATER 6
#define OUTSIDE_LAMP_PIN 7
#define CLK 9
#define DT 10
#define SW 11
#define HEATER_PIN 12
#define SETPOINT_PIN 0

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
DS3231 rtc(SDA, SCL);
OneWire tempInside(TEMP_INSIDE);
DallasTemperature sensorInside(&tempInside);
OneWire tempOutside(TEMP_OUTSIDE);
DallasTemperature sensorOutside(&tempOutside);
OneWire tempWater(TEMP_WATER);
DallasTemperature sensorWater(&tempWater);
Encoder enc1(CLK, DT, SW);
OneButton upButton(UP_PIN, true);
OneButton downButton(DOWN_PIN, true);

/////// Переменные для запроса температуры.
//Температура с датчика DS18B20.
float temperatureInside;
float temperatureOutside;
float temperatureWater;
//Предыдущее время запроса температуры.
unsigned long previousTimeTemperatureRequest;
//Предыдущее время чтения температуры.
unsigned long previousTimeTemperatureRead;

///////Переменные для времени.
//Время и дата.
String currentDate;
String currentTime;
Time t;
//Предыдущее время считывания даты и времени.
unsigned long previousTimeDataTimeRead;

//Уставки температуры в градусах Цельсия.
float manualModeSetPoint = 21;               //            ---------------------- toEEPROM
float daySetPoint = 19;                     //      ---------------------- toEEPROM
float nightSetPoint = 23;                   //        ---------------------- toEEPROM
//Состояние обогревателя, true - включен.
bool heaterStatus = true;
//Нужно нагреть.
bool needWarm;
//Нужно охладить.
bool needCool; 
//Начало временной зоны 1.
int zoneOneBegin = 9;                   //            ---------------------- toEEPROM
//Конец временной зоны 1.
int zoneOneEnd = 20;                     //            ---------------------- toEEPROM

float deadZoneValue = 0.4;                     //      ---------------------- toEEPROM
float oneSideDeadZoneValue = deadZoneValue/2;

///////Переменные для переключения режимов отопления.
//Номер режима.
int modeNumber = 1;                          // ---------------------- toEEPROM
//Количество режимов всего.
const int totalModeNumber = 3;

//Предыдущее время обновления экрана.
unsigned long previousTimeScreenRefresh;

//Предыдущее время отправки данных.
unsigned long previousTimeSendDataToSerial;

//Время цикла loop.
int loopCycleTime;

// Редактируемая на OLED уставка.
int editableSetPoint = 1;
// Количество уставок.
int setPointCount = 3;

//Режим уличного фонаря.
int outsideLampMode = 1;                           // ---------------------- toEEPROM
bool outsideLampState;
int currentOutsideLampInterval;

void setup()
{
  //Выключаем ватчдог после перезагрузки.
  wdt_disable();
  
  //Подождать запуска Wemos.
  delay(1000);

  //Uart setup.
  Serial.begin(115200);
  
  //Time setup.                                       
  rtc.begin();
  previousTimeDataTimeRead = 0;
  //The following lines can be uncommented to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(15, 37, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(5, 10, 2018);   // Set the date to January 1st, 2014

  //Temperature setup.
  sensorInside.begin();
  sensorOutside.begin();
  sensorWater.begin();
  sensorInside.requestTemperatures();
  sensorOutside.requestTemperatures();
  sensorWater.requestTemperatures();
  temperatureInside = sensorInside.getTempCByIndex(0);
  temperatureOutside = sensorOutside.getTempCByIndex(0);
  temperatureWater = sensorWater.getTempCByIndex(0);
  previousTimeTemperatureRead = 0;
  previousTimeTemperatureRequest = 0;
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);

  //OLED setup
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_victoriabold8_r);

  //Encoder setup.
  enc1.setType(TYPE2);

  //Buttons setup.
  upButton.attachClick(editableSetPointPrev);
  downButton.attachClick(editableSetPointNext);
  
  //Интервал, через который ватчдог сбросит МК, если таймер ватчдога не обнулится.
  wdt_enable (WDTO_2S);
}

void loop()
{   
  int loopCycleBegin = millis();

  //Опрос энкодера.
  enc1.tick();

  //Опрос кнопок навигации по меню.
  upButton.tick();
  downButton.tick();
  
  requestTemperature(30000);

  getTemperature(31000);

  getDateTime(1000);
  
  heaterManage();

  manageOutsideLamp(1000, 100);
    
  printScreen(400);
  
  modeSwitching();

  manageSetPoints();

  sendDataToSerial(1000);
 
  receiveDataFromSerial();
  
  wdt_reset();
  
  loopCycleTime = millis() - loopCycleBegin;
}
