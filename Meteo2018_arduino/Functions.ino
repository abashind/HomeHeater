void requestTemperature(int waitInterval)
{
  if ((millis() - previousTimeTemperatureRequest) >= waitInterval)
  {
    sensorInside.requestTemperatures();
    sensorOutside.requestTemperatures();
    previousTimeTemperatureRequest = millis();
  }
}

void getTemperature(int waitInterval)
{
  if ((millis() - previousTimeTemperatureRead) >= waitInterval)
  {
    temperatureInside = sensorInside.getTempCByIndex(0);
    temperatureOutside = sensorOutside.getTempCByIndex(0);
    previousTimeTemperatureRead = millis();
  }
}

void getDateTime(int waitTime)
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
    u8x8.print("SetPoint: " + String(manualModeSetPoint));
    u8x8.setCursor(0,5);
    u8x8.print("LoopTime: " + String(loopCycleTime));
    previousTimeScreenRefresh = millis();
  }
}

void heaterManage()
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
  if (enc1.isRight()) manualModeSetPoint += 0.2;
  if (enc1.isLeft()) manualModeSetPoint -= 0.2;
}

void sendDataToSerial(int waitTime)
{
  if((millis() - previousTimeSendDataToSerial) >= waitTime)
  {   
    StaticJsonBuffer<450> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["tempIn"] = temperatureInside;
    root["tempOut"] = temperatureOutside;
    root["time"] = currentTime;
    root["date"] = currentDate;
    root["manSetPoint"] = manualModeSetPoint;
    root["heatSt"] = heaterStatus;
    root["modeNumber"] = modeNumber;
    root.printTo(Serial);
    
    previousTimeSendDataToSerial = millis();
  }
}

void receiveJsonDataFromSerial()
{
  if (Serial.available())
  {
    StaticJsonBuffer<450> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(Serial);
    manualModeSetPoint = root["manSetPoint"];
    daySetPoint = root["daySetPoint"];
    nightSetPoint = root["nightSetPoint"];
    modeNumber = root["modeNumber"];
  }
}
