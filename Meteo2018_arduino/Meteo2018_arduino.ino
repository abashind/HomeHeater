//Управление обогревом, Абашин Дмитрий.
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
#define SCREEN_BUTTON_PIN 4
#define HEATER_PIN 12
#define HEARTBIT_PIN 13
#define OLED_RESET 4

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
DS3231 rtc(SDA, SCL);
OneWire tempInside(TEMP_INSIDE);
DallasTemperature sensorInside(&tempInside);
OneWire tempOutside(TEMP_OUTSIDE);
DallasTemperature sensorOutside(&tempOutside);

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

/////// Переменные для управления обогревателем.
//Уставка температуры в вольтах.
float setPointRaw;
//Уставка температуры в градусах Цельсия.
float setPointCelsius;
//Состояние обогревателя, true - включен.
bool heaterStatus = true;

///////Переменные для переключения экрана.
//Нажата ли кнопка переключения экранов.
bool screenButtonWasUp = true;
//Номер экрана
int screenNumber = 1;
//Количество экранов всего.
const int totalScreenNumber = 5;
//Предыдущее время обновления экрана.
unsigned long previousTimeScreenRefresh;

///////Переменные для пульса.
//Предыдущее время пульса.
unsigned long previousTimeHeartbit;

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
  pinMode(SCREEN_BUTTON_PIN, INPUT_PULLUP);

  //OLED setup
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_victoriabold8_r);
  
  //Интервал, через который ватчдог сбросит МК, если таймер ватчдога не обнулится.
  wdt_enable (WDTO_2S);

}

void loop()
{   
    int loopCycleBegin = millis();
    
    getTemperatures(60000);

    readTime(800);

    heaterManage();
      
    printScreen1(1300);
    
    modeSwitching();

	  sendDataToSerial(500);
    
    wdt_reset();
    
    loopCycleTime = millis() - loopCycleBegin;
}

void getTemperatures(int waitInterval)
{
  if ((millis() - previousTimeTemperatureRequest) >= waitInterval)
  {
    sensorInside.requestTemperatures();
    sensorOutside.requestTemperatures();
    previousTimeTemperatureRequest = millis();
    delay(100);
    temperatureInside = sensorInside.getTempCByIndex(0);
    temperatureOutside = sensorOutside.getTempCByIndex(0);
    previousTimeTemperatureRead = millis();
  }
}

void readTime(int waitTime)
{
  if ((millis() - previousTimeDataTimeRead) >= waitTime)
    {       
      currentDate = rtc.getDateStr(FORMAT_SHORT);
      currentTime = rtc.getTimeStr();
      previousTimeDataTimeRead = millis();
    }
}

void printScreen1(int waitTime)
{
  if ((millis() - previousTimeScreenRefresh) >= waitTime)
  {
    u8x8.setCursor(0,0);
    u8x8.print(currentDate);
    u8x8.setCursor(0,1);
    u8x8.print(currentTime);
    u8x8.setCursor(0,2);
    u8x8.print("TempIn: " + String(temperatureInside));
    u8x8.setCursor(0,3);
    u8x8.print("TempOut: " + String(temperatureOutside));
    u8x8.setCursor(0,4);
    u8x8.print("SetPoint: " + String(setPointCelsius));
    u8x8.setCursor(0,5);
    u8x8.print("LoopTime: " + String(loopCycleTime));
    previousTimeScreenRefresh = millis();
  }
}

void heaterManage()
{
    setPointRaw = analogRead(SETPOINT_PIN);
    setPointCelsius = map(setPointRaw, 0, 1023, 0, 270);
    setPointCelsius = setPointCelsius/10;
    if (setPointCelsius >= temperatureInside) 
    {
      digitalWrite(HEATER_PIN, HIGH);
      heaterStatus = true;
    }    
    else
    {
      digitalWrite(HEATER_PIN, LOW);
      heaterStatus = false;
    }
}

void modeSwitching()
{
  if (screenButtonWasUp && !digitalRead(SCREEN_BUTTON_PIN)) 
    {
      delay(10);
      if (!digitalRead(SCREEN_BUTTON_PIN))
      {
        if(screenNumber < totalScreenNumber)
          {screenNumber ++;}
        else
          {screenNumber = 1;}
      }
    }
    screenButtonWasUp = digitalRead(SCREEN_BUTTON_PIN);
}

void sendDataToSerial(int waitTime)
{
  if((millis() - previousTimeHeartbit) >= waitTime)
  {   
    StaticJsonBuffer<450> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["tempIn"] = temperatureInside;
    root["tempOut"] = temperatureOutside;
    root["time"] = currentTime;
    root["date"] = currentDate;
    root["setPoint"] = setPointCelsius;
    root["heatSt"] = heaterStatus;
    root["screen"] = screenNumber;
    root.printTo(Serial);
    Serial.println();
    jsonBuffer.clear();

    previousTimeHeartbit = millis();
  }
}
