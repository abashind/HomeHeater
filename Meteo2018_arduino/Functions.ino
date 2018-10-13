void requestTemperature(int waitInterval)
{
  if ((millis() - previousTimeTemperatureRequest) >= waitInterval)
  {
    sensorInside.requestTemperatures();
    sensorOutside.requestTemperatures();
    sensorWater.requestTemperatures();
    previousTimeTemperatureRequest = millis();
  }
}

void getTemperature(int waitInterval)
{
  if ((millis() - previousTimeTemperatureRead) >= waitInterval)
  {
    temperatureInside = sensorInside.getTempCByIndex(0);
    temperatureOutside = sensorOutside.getTempCByIndex(0);
    temperatureWater = sensorWater.getTempCByIndex(0);
    previousTimeTemperatureRead = millis();
  }
}

void getDateTime(int waitTime)
{
  if ((millis() - previousTimeDataTimeRead) >= waitTime)
    {       
      currentDate = rtc.getDateStr(FORMAT_SHORT);
      currentTime = rtc.getTimeStr();
      t = rtc.getTime();
      previousTimeDataTimeRead = millis();
    }
}

void printScreen1(int waitTime)
{
  if ((millis() - previousTimeScreenRefresh) >= waitTime)
  {
    const char* tIn;
    const char* tOut;
    const char* tW;
    const char* mSP;
    const char* dSP;
    const char* nSP;
    dtostrf(temperatureInside,6,1,tIn);
    dtostrf(temperatureOutside,6,1,tOut);
    dtostrf(temperatureWater,6,1,tW);
    dtostrf(manualModeSetPoint,6,1,mSP);
    dtostrf(daySetPoint,6,1,dSP);
    dtostrf(nightSetPoint,6,1,nSP);
    
    u8x8.setCursor(0,0);
    u8x8.print(currentTime);
    u8x8.setCursor(10,0);
    u8x8.print("mode:" + String(modeNumber));
    
    u8x8.setCursor(0,1);
    u8x8.print("tI:" + String(tIn));
    u8x8.setCursor(8,1);
    u8x8.print("tO:" + String(tOut));
    
    u8x8.setCursor(0,2);
    u8x8.print("tW: " + String(tW));
    u8x8.setCursor(8,2);
    u8x8.print("mT: " + String(mSP));

    u8x8.setCursor(0,3);
    u8x8.print("dT: " + String(dSP));
    u8x8.setCursor(8,3);
    u8x8.print("nT: " + String(nSP));
    
    u8x8.setCursor(0,4);
    u8x8.print("lT: " + String(loopCycleTime));
    previousTimeScreenRefresh = millis();
  }
}

void heaterManage()
{
  switch (modeNumber)
  {
    //Ручной режим.
    case 1:
    {
      if (manualModeSetPoint >= temperatureInside) 
        {
          digitalWrite(HEATER_PIN, HIGH);
          heaterStatus = true;
        }    
      else
        {
          digitalWrite(HEATER_PIN, LOW);
          heaterStatus = false;
        }
      break;
    }

    //Режим день/ночь.
    case 2:
    {
      //День. 
      if (t.hour >= 9 && t.hour <= 20 )
      {
        if (daySetPoint >= temperatureInside) 
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
      
      //Ночь.
      else
      {
        if (nightSetPoint >= temperatureInside) 
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
    }

    //Автоматика отключена.
    case 3:
    {
      if(heaterStatus)
        digitalWrite(HEATER_PIN, HIGH);
      else
        digitalWrite(HEATER_PIN, LOW);
      break;
    }
  }
}

void modeSwitching()
{
  if (enc1.isRelease())
      {
        if(modeNumber < totalModeNumber)
          {modeNumber ++;}
        else
          {modeNumber = 1;}
      }
}

void manageSetPointInManualMode()
{
  if (enc1.isRight()) manualModeSetPoint += 0.5;
  if (enc1.isLeft()) manualModeSetPoint -= 0.5;
}

void sendDataToSerial(int waitTime)
{
  if((millis() - previousTimeSendDataToSerial) >= waitTime)
  {   
    StaticJsonBuffer<650> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["tempIn"] = temperatureInside;
    root["tempOut"] = temperatureOutside;
    root["tempW"] = temperatureWater;
    root["time"] = currentTime;
    root["date"] = currentDate;
    root["manSetPoint"] = manualModeSetPoint;
    root["heatSt"] = heaterStatus;
    root["daySetPoint"] = daySetPoint;
    root["nightSetPoint"] = nightSetPoint;
    root["modeNumber"] = modeNumber;
    root.printTo(Serial);
    Serial.println();
    previousTimeSendDataToSerial = millis();
  }
}

void receiveDataFromSerial()
{
  if(Serial.available() > 0 )
  {
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(Serial);
    if(root["manSetPoint"] != 0)
    {manualModeSetPoint = root["manSetPoint"];}
    if(root["modeNumber"] != 0) 
    {modeNumber = root["modeNumber"];}
    if(root["daySetPoint"] != 0)
    {daySetPoint = root["daySetPoint"];}
    if(root["nightSetPoint"] != 0)
    {nightSetPoint = root["nightSetPoint"];}
  }
}
