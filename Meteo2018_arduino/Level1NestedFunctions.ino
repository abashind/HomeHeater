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
    float outTemp = sensorOutside.getTempCByIndex(0);
    if(int(outTemp) != - 127)
      temperatureOutside = outTemp;
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

void printScreen(int waitTime)
{
  if ((millis() - previousTimeScreenRefresh) >= waitTime)
  { 
    u8x8.setCursor(0,0);
    u8x8.print(currentTime);
    u8x8.setCursor(9,0);
    u8x8.print("mode:" + String(modeNumber));
    
    u8x8.setCursor(0,1);
    u8x8.print("tI:" + String(temperatureInside).substring(0,5));
    u8x8.setCursor(8,1);
    u8x8.print("tO:" + String(temperatureOutside).substring(0,5));
    
    u8x8.setCursor(0,2);
    u8x8.print("tW:" + String(temperatureWater).substring(0,5));
    u8x8.setCursor(8,2);
    if(editableSetPoint == 1)
    {
    u8x8.setFont(u8x8_font_artosserif8_r);
    u8x8.print("mT:" + String(manualModeSetPoint).substring(0,5));
    u8x8.setFont(u8x8_font_victoriabold8_r);
    }
    else
    {
      u8x8.print("mT:" + String(manualModeSetPoint).substring(0,5));
    }
    
    u8x8.setCursor(0,3);
    if(editableSetPoint == 2)
    {
      u8x8.setFont(u8x8_font_artosserif8_r);
      u8x8.print("dT:" + String(daySetPoint).substring(0,5));
      u8x8.setFont(u8x8_font_victoriabold8_r);
    }
    else
    {
      u8x8.print("dT:" + String(daySetPoint).substring(0,5));
    }
    u8x8.setCursor(8,3);
    if(editableSetPoint == 3)
    {
      u8x8.setFont(u8x8_font_artosserif8_r);
      u8x8.print("nT:" + String(nightSetPoint).substring(0,5));
      u8x8.setFont(u8x8_font_victoriabold8_r);
    }
    else
    {
      u8x8.print("nT:" + String(nightSetPoint).substring(0,5));
    }
    u8x8.setCursor(0,4);
    u8x8.print("lT:" + String(loopCycleTime));
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
      warmCool(manualModeSetPoint);    
      break;
    }

    //Режим день/ночь.
    case 2:
    {
      //День. 
      if (t.hour >= zoneOneBegin && t.hour <= zoneOneEnd )
        warmCool(daySetPoint);      
      //Ночь.
      else
        warmCool(nightSetPoint);
      break;
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

void manageSetPoints()
{
  switch(editableSetPoint)
  {
    case 1:
    {
      if (enc1.isRight()) manualModeSetPoint += 0.5;
      if (enc1.isLeft()) manualModeSetPoint -= 0.5;
      break;
    }
    
    case 2:
    {
      if (enc1.isRight()) daySetPoint += 0.5;
      if (enc1.isLeft()) daySetPoint -= 0.5;
      break;
    }
    
    case 3:
    {
      if (enc1.isRight()) nightSetPoint += 0.5;
      if (enc1.isLeft()) nightSetPoint -= 0.5;
      break;
    }
  }
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
      manualModeSetPoint = root["manSetPoint"];
    if(root["modeNumber"] != 0) 
      modeNumber = root["modeNumber"];
    if(root["daySetPoint"] != 0)
      daySetPoint = root["daySetPoint"];
    if(root["nightSetPoint"] != 0)
      nightSetPoint = root["nightSetPoint"];
    if(root.containsKey("heatSt"))
      heaterStatus = root["heatSt"];
  }
}

void editableSetPointPrev()
{
  if(editableSetPoint > 1)
    editableSetPoint--;
}

void editableSetPointNext()
{
  if(editableSetPoint < 3)
    editableSetPoint++;
}

void manageOutsideLamp(int blynkInterval, int strobeInterval)
{
  switch(outsideLampMode)
  {
    //Уличный фонарь выключен.
    case 1:
    {
      digitalWrite(OUTSIDE_LAMP_PIN, LOW);
      outsideLampState = false;
      break;
    }
    //Уличный фонарь включен.
    case 2:
    {
      digitalWrite(OUTSIDE_LAMP_PIN, HIGH);
      outsideLampState = true;
      break;
    }
    //Уличный фонарь мигает.
    case 3:
    {
      outsideLampBlynk(blynkInterval);
    }
    case 4:
    {
      outsideLampBlynk(strobeInterval);
    }
  }
}


