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
