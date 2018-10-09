#include <avr/wdt.h>

void setup() {
  wdt_disable(); // бесполезная строка до которой не доходит выполнение при bootloop
  Serial.begin(9600);
  Serial.println("Setup..");
  
  Serial.println("Wait 5 sec..");
  delay(5000); // Задержка, чтобы было время перепрошить устройство в случае bootloop
  wdt_enable (WDTO_8S); // Для тестов не рекомендуется устанавливать значение менее 8 сек.
  Serial.println("Watchdog enabled.");
}

int timer = 0;

void loop(){
  // Каждую секунду мигаем светодиодом и значение счетчика пишем в Serial
  if(!(millis()%1000)){
    timer++;
    Serial.println(timer);
    digitalWrite(13, digitalRead(13)==1?0:1); delay(1);
  }
//  wdt_reset();
}
