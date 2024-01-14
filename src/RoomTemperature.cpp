#include <Arduino.h>
#include "RoomTemperature.h"

RoomTemperature::RoomTemperature(DHT_Unified &dht)
    : dht(dht)
{
}


void RoomTemperature::initilize()
{
  dht.begin();
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
}

double RoomTemperature::readTemperature()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    // Serial.println("Error reading temperature!");
  }
  else
  {
    roomTemp = event.temperature;
  }
  return roomTemp;
}

double RoomTemperature::getTemperature()
{
  return roomTemp;
}