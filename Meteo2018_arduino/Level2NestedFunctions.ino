void warmCool(float setPoint)
{
  //Если холодно и температура воды ниже уставки, включить котёл.
  if(temperatureInside <= setPoint - insideTempOneSideDeadZone && temperatureWater < waterSetPoint - waterTempOneSideDeadZone)
  {
    digitalWrite(HEATER_PIN, HIGH);
    heaterStatus = true; 
  }
 //Если жарко или температура воды выше уставки, ВЫключить котёл.
 if(temperatureInside >= setPoint + insideTempOneSideDeadZone || temperatureWater >= waterSetPoint + waterTempOneSideDeadZone)
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
