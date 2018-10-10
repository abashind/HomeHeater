#include <GyverEncoder.h>
#include <Arduino.h>
#include <U8x8lib.h>
#include <Wire.h>
#include <DS3231.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>
#include <ArduinoJson.h>

#define TEMP_INSIDE 3
#define TEMP_OUTSIDE 2
#define SETPOINT_PIN 0
#define UP_PIN 4
#define DOWN_PIN 5
#define HEATER_PIN 12
#define CLK 9
#define DT 10
#define SW 11

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
DS3231 rtc(SDA, SCL);
OneWire tempInside(TEMP_INSIDE);
DallasTemperature sensorInside(&tempInside);
OneWire tempOutside(TEMP_OUTSIDE);
DallasTemperature sensorOutside(&tempOutside);
Encoder enc1(CLK, DT, SW);

/////// Переменные для запроса температуры.
//Температура с датчика DS18B20.
float temperatureInside;
float temperatureOutside;
//Предыдущее время запроса температуры.
unsigned long previousTimeTemperatureRequest;
//Предыдущее время чтения температуры.
unsigned long previousTimeTemperatureRead;

///////Переменные для времени.
//Время и дата.
String currentDate;
String currentTime;
//Предыдущее время считывания даты и времени.
unsigned long previousTimeDataTimeRead;

//Уставка температуры в градусах Цельсия.
float manualModeSetPoint = 21;
float daySetPoint = 19;
float nightSetPoint = 23;
//Состояние обогревателя, true - включен.
bool heaterStatus = true;

///////Переменные для переключения режимов.
//Номер экрана
int modeNumber = 1;
//Количество экранов всего.
const int totalModeNumber = 5;

//Предыдущее время обновления экрана.
unsigned long previousTimeScreenRefresh;

//Предыдущее время отправки данных.
unsigned long previousTimeSendDataToSerial;

//Время цикла loop.
int loopCycleTime;

void setup()
{
  //Выключаем ватчдог после перезагрузки.
  wdt_disable();
  
  //Подождать запуска Wemos.
  delay(3000);

  //Uart setup.
  Serial.begin(9600);
  
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
  sensorInside.requestTemperatures();
  sensorOutside.requestTemperatures();
  temperatureInside = sensorInside.getTempCByIndex(0);
  temperatureOutside = sensorOutside.getTempCByIndex(0);
  previousTimeTemperatureRead = 0;
  previousTimeTemperatureRequest = 0;
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(UP_PIN, INPUT_PULLUP);

  //OLED setup
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_victoriabold8_r);

  //Encoder setup.
  enc1.setType(TYPE2);
  
  //Интервал, через который ватчдог сбросит МК, если таймер ватчдога не обнулится.
  wdt_enable (WDTO_2S);

}

void loop()
{   
    //Опрос энкодера.
    enc1.tick();
    
    int loopCycleBegin = millis();
    
    requestTemperature(30000);

    getTemperature(31000);

    getDateTime(1000);

    manageSetPointInManualMode();
    
    heaterManage();
      
    printScreen1(400);
    
    modeSwitching();

	  sendDataToSerial(500);

    delay(20);
    
    receiveJsonDataFromSerial();
    
    wdt_reset();
    
    loopCycleTime = millis() - loopCycleBegin;
}
