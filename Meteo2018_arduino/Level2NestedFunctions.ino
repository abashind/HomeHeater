void warmCool(float setPoint)
{
  //Если холодно, включить котёл.
  if(temperatureInside <= setPoint - oneSideDeadZoneValue)
    {
      needWarm = true;
      needCool = false;
    }   
  if(needWarm)
  {
    digitalWrite(HEATER_PIN, HIGH);
    heaterStatus = true;
    if(temperatureInside >= setPoint)
    {
      needWarm = false;
      digitalWrite(HEATER_PIN, LOW);
      heaterStatus = false;
    }    
  }

  //Если жарко, ВЫключить котёл.
  if(temperatureInside >= setPoint + oneSideDeadZoneValue)
  {
    needCool = true;
  }
  if(needCool)
  {
    digitalWrite(HEATER_PIN, LOW);
    heaterStatus = false;
  }
}

void outsideLampBlynk(int interval)
{
  if ((millis() - currentOutsideLampInterval) >= interval)
  {
    if(outsideLampState)
    {
      digitalWrite(OUTSIDE_LAMP_PIN, LOW);
      outsideLampState = false;
    }
    else
    {
      digitalWrite(OUTSIDE_LAMP_PIN, HIGH);
      outsideLampState = true;
    }
    currentOutsideLampInterval = millis();
  }
}
