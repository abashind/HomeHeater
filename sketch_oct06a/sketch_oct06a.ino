#include <Wire.h>
#include <iarduino_RTC.h>  
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>
//#include <OLED_I2C.h>
#include <SoftwareSerial.h>
#include <CayenneMQTTESP8266Shield.h>

#define TEMP_INSIDE 3
#define TEMP_OUTSIDE 2
#define SETPOINT_PIN 0
#define SCREEN_BUTTON_PIN 4
#define HEATER_PIN 12
#define HEARTBIT_PIN 13

iarduino_RTC time(RTC_DS3231); 
OneWire tempInside(TEMP_INSIDE);
DallasTemperature sensorInside(&tempInside);
OneWire tempOutside(TEMP_OUTSIDE);
DallasTemperature sensorOutside(&tempOutside);
//OLED  myOLED(SDA, SCL);
SoftwareSerial EspSerial(9, 10); // RX, TX
ESP8266 wifi(&EspSerial);

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

char ssid[] = "7SkyHome";
char wifiPassword[] = "89191532537";
char username[] = "d9135cf0-c958-11e8-9555-33faa5c5b5a8";
char password[] = "b8d40406e53066411fb13d3192c32ca6d3d8f7c8";
char clientID[] = "b8d40406e53066411fb13d3192c32ca6d3d8f7c8";


void setup() 
{
  //Выключаем ватчдог после перезагрузки.
  wdt_disable();
  
  Serial.begin(9600);

  //Time setup.
  delay(1000);                                            
  time.begin();
  previousTimeDataTimeRead = 0;

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
  //if(!myOLED.begin(SSD1306_128X64))
    //while(1);   // In case the library failed to allocate enough RAM for the display buffer...  
  //myOLED.setFont(SmallFont);
  //myOLED.print("EbanaRot!!!", LEFT, 0);
  //myOLED.update();
  //Интервал, через который ватчдог сбросит МК, если таймер ватчдога не обнулится.
  wdt_enable (WDTO_2S);

  EspSerial.begin(9600);
  delay(10);
  Cayenne.begin(username, password, clientID, wifi, ssid, wifiPassword);
}


void loop() 
{
  pulse(1000);
  readTime(1000);
  // Сброс таймера ватчдога.
  wdt_reset();
  Cayenne.loop();
  
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
    Serial.println("All Ok!!");
    previousTimeHeartbit = millis();
  }
}

void readTime(int waitTime)
{
  if ((millis() - previousTimeDataTimeRead) >= waitTime)
    {       
      currentDateTime = time.gettime("d-m-y, H:i:s, w");
      Serial.println(currentDateTime);
      previousTimeDataTimeRead = millis();
    }
}

CAYENNE_OUT_DEFAULT()
{
  // Write data to Cayenne here. This example just sends the current uptime in milliseconds on virtual channel 0.
  Cayenne.virtualWrite(0, millis());
  // Some examples of other functions you can use to send data.
  //Cayenne.celsiusWrite(1, 22.0);
  //Cayenne.luxWrite(2, 700);
  //Cayenne.virtualWrite(3, 50, TYPE_PROXIMITY, UNIT_CENTIMETER);
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
