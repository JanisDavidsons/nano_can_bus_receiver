#ifndef ROOM_TEMPERATURE
#define ROOM_TEMPERATURE

#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 3
#define DHTTYPE DHT11      

class RoomTemperature
{
private:
  DHT_Unified &dht;
  double roomTemp = 0.0;
  sensor_t sensor;

public:
  RoomTemperature(DHT_Unified &dht);
  void initilize();
  double readTemperature();
  double getTemperature();
};

#endif