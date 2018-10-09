//Управление обогревом, Абашин Дмитрий.
#include <Wire.h>
#include <DS3231.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>
#include <OLED_I2C.h>
#include <ArduinoJson.h>

#define TEMP_INSIDE 3
#define TEMP_OUTSIDE 2
#define SETPOINT_PIN 0
#define SCREEN_BUTTON_PIN 4
#define HEATER_PIN 12
#define HEARTBIT_PIN 13

DS3231 rtc(SDA, SCL);
OneWire tempInside(TEMP_INSIDE);
DallasTemperature sensorInside(&tempInside);
OneWire tempOutside(TEMP_OUTSIDE);
DallasTemperature sensorOutside(&tempOutside);
OLED  myOLED(SDA, SCL);

/////// Переменные для запроса температуры.
//Температура с датчика DS18B20.
int temperatureInside;
int temperatureOutside;
//Предыдущее время запроса температуры.
unsigned long previousTimeTemperatureRequest;
//Предыдущее время чтения температуры.
unsigned long previousTimeTemperatureRead;

///////Переменные для времени.
//Время и дата.
String currentDateTime;
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
//Предыдущее состояние пина
bool heartbitValue = LOW;

// Шрифт для OLED
extern uint8_t SmallFont[];

void setup()
{
  //Выключаем ватчдог после перезагрузки.
  wdt_disable();

  Serial.begin(9600);
  
  //Time setup.
  delay(1000);                                            
  rtc.begin();
  previousTimeDataTimeRead = 0;
  // The following lines can be uncommented to set the date and time
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

  //Интервал, через который ватчдог сбросит МК, если таймер ватчдога не обнулится.
  wdt_enable (WDTO_2S);

  //OLED setup
  if(!myOLED.begin(SSD1306_128X64))
    while(1);   // In case the library failed to allocate enough RAM for the display buffer...  
  myOLED.setFont(SmallFont);
}

void loop()
{   
    //Запрашиваем температуру раз в 50 секунд.
    requestTemp(50000);

    //Читаем температуру раз в минуту.
    readTemp(60000);

    //Читаем время 2 раза в секунду.
    readTime(500);

    //Читаем уставку и управляем обогревателем - постоянно.
    heaterManage();
      
    //Обновляем экран.
    printScreen1();
    
    //Переключаем экраны - постоянно
    screenSwitching();
	
	  //Отправка Json данных в сериал.
	  pulse(500);
    
    // Сброс таймера ватчдога.
    wdt_reset();
}

void requestTemp(int waitInterval)
{
  if ((millis() - previousTimeTemperatureRequest) >= waitInterval)
  {
    sensorInside.setWaitForConversion(false);
    sensorInside.requestTemperatures();
    sensorInside.setWaitForConversion(true);
    sensorOutside.setWaitForConversion(false);
    sensorOutside.requestTemperatures();
    sensorOutside.setWaitForConversion(true);
    previousTimeTemperatureRequest = millis();
  }
    
}

void readTemp(int waitInterval)
{
    if ((millis() - previousTimeTemperatureRead) >= waitInterval)
    {
      temperatureInside = sensorInside.getTempCByIndex(0);
      temperatureOutside = sensorOutside.getTempCByIndex(0);
      previousTimeTemperatureRead = millis();
    }
}

void readTime(int waitTime)
{
  if ((millis() - previousTimeDataTimeRead) >= waitTime)
    {       
      currentDateTime = String(rtc.getDateStr(FORMAT_SHORT)) + " " + String(rtc.getTimeStr());
      previousTimeDataTimeRead = millis();
    }
}

void printScreen1()
{
  myOLED.clrScr();
  myOLED.print(currentDateTime, LEFT, 0);
  myOLED.print("Temp inside: " + String(temperatureInside), LEFT, 10);
  myOLED.print("Temp outside: " + String(temperatureOutside), LEFT, 20);
  myOLED.print("SetPoint: " + String(setPointCelsius), LEFT, 30);
  myOLED.update();
  previousTimeScreenRefresh = millis();
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

void screenSwitching()
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

void pulse(int waitTime)
{
  if((millis() - previousTimeHeartbit) >= waitTime)
  {
    if(heartbitValue == LOW)
      heartbitValue = HIGH;
    else
      heartbitValue = LOW;
    
    digitalWrite(HEARTBIT_PIN, heartbitValue);
    
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["tempIn"] = temperatureInside;
    root["tempOut"] = temperatureOutside;
    root["dateTime"] = currentDateTime;
    root["setPoint"] = setPointCelsius;
    root["heatSt"] = heaterStatus;
    root["screen"] = screenNumber;
    root.printTo(Serial);
    Serial.println();
    jsonBuffer.clear();

    previousTimeHeartbit = millis();
    
  }
}
