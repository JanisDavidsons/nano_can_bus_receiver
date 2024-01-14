// CanBusReceiver.cpp

#include "CanBusReceiver.h"

CanBusReceiver::CanBusReceiver(MCP2515 &mcp2515)
    : mcp2515(mcp2515),
      trendExhaust(TREND_EXHAUST_NOT_CHANGING),
      voltageChanged(true)
{
}

void CanBusReceiver::copyCanMsgToStruct(const struct can_frame &canMsg, struct frameFlameSensor &flameSensorData)
{
  flameSensorData.can_id = canMsg.can_id;
  flameSensorData.can_dlc = canMsg.can_dlc;

  for (int i = 0; i < flameSensorData.can_dlc; ++i)
  {
    flameSensorData.data[i] = canMsg.data[i];
  }
}

void CanBusReceiver::copyCanMsgToStruct(const struct can_frame &canMsg, struct frameVoltageSensor &voltageSensorData)
{
  voltageSensorData.can_id = canMsg.can_id;
  voltageSensorData.can_dlc = canMsg.can_dlc;

  for (int i = 0; i < voltageSensorData.can_dlc; ++i)
  {
    voltageSensorData.data[i] = canMsg.data[i];
  }
}

void CanBusReceiver::processFrameVoltageSensor(frameVoltageSensor &sensorData)
{
  // for (size_t i = 0; i < sizeof(sensorData.data); i++)
  // {
  //   Serial.print("[");
  //   Serial.print(sensorData.data[i]);
  //   Serial.print("]");
  // }
  // Serial.println("");

  uint16_t reconstructedValue =
      (static_cast<uint8_t>(sensorData.data[0])) |
      (static_cast<uint8_t>(sensorData.data[1]) << 8);

  int16_t reconstructedTemperature =
      (static_cast<int8_t>(sensorData.data[2])) |
      (static_cast<int8_t>(sensorData.data[3]) << 8);

  double newTemperature = static_cast<double>(reconstructedTemperature) / 100;
  double newVoltage = static_cast<double>(reconstructedValue) / 100;

  sensorData.temperature = newTemperature;

  // Serial.print("Received message with ID: 0x");
  // Serial.println(sensorData.can_id, HEX);
  // Serial.print("Voltage: ");
  // Serial.println(sensorData.voltage);
  // Serial.print("Temperature: ");
  // Serial.println(sensorData.temperature);

  if (newVoltage != sensorData.voltage)
  {
    sensorData.voltage = newVoltage;
    voltageChanged = true;
    lastMessageTime = millis(); // Update last message time
  }
}

void CanBusReceiver::processFrameFlameSensor(frameFlameSensor &sensorData)
{
  // for (int i = 0; i < sensorData.can_dlc; ++i)
  // {
  //   Serial.print(sensorData.data[i]);
  //   Serial.print(' ');
  // }
  // Serial.println();

  int16_t reconstructedTemperature =
      (sensorData.data[0]) |
      (sensorData.data[1] << 8);

  double newTemperature = static_cast<double>(reconstructedTemperature) / 100;

  sensorData.value = newTemperature;
  sensorData.isIncreasing = static_cast<bool>(sensorData.data[2]);
  sensorData.isDecreasing = static_cast<bool>(sensorData.data[3]);

  setTrend(sensorData);
}

void CanBusReceiver::checkMessage()
{
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)
  {
    if (canMsg.can_id == 0x100)
    {
      if (canMsg.can_dlc == sizeof(exhaustTemperature.data))
      {
        copyCanMsgToStruct(canMsg, exhaustTemperature);
        processFrameFlameSensor(exhaustTemperature);
      }
      else
      {
        Serial.println("Error: Size mismatch in received 0x100 message");
      }
    }
    else if (canMsg.can_id == 0x101)
    {
      if (canMsg.can_dlc == sizeof(voltageSensor.data))
      {
        copyCanMsgToStruct(canMsg, voltageSensor);
        processFrameVoltageSensor(voltageSensor);
      }
      else
      {
        Serial.println("Error: Size mismatch in received 0x101 message");
      }
    }

    lastMessageTime = millis();
  }
}

unsigned long CanBusReceiver::getLatMessageTime()
{
  return lastMessageTime;
}

double CanBusReceiver::getExhaustTemp() const
{
  return exhaustTemperature.value;
}

const struct can_frame &CanBusReceiver::getCanMsg() const
{
  return canMsg;
}

double CanBusReceiver::getTemperature() const
{
  return voltageSensor.temperature;
}

bool CanBusReceiver::isExhaustTempIncreasing()
{
  return exhaustTemperature.isIncreasing;
}

bool CanBusReceiver::isExhaustTempDecreasing()
{
  return exhaustTemperature.isDecreasing;
}

double CanBusReceiver::getVoltage() const
{
  return voltageSensor.voltage;
}

void CanBusReceiver::setTrend(frameFlameSensor &sensor)
{
  if (sensor.isIncreasing)
  {
    trendExhaust = TREND_EXHAUST_RISING;
  }
  else if (sensor.isDecreasing)
  {
    trendExhaust = TREND_EXHAUST_DECREASING;
  }
  else
  {
    trendExhaust = TREND_EXHAUST_NOT_CHANGING;
  }
}

CanBusReceiver::Trend CanBusReceiver::getTExhaustTrend()
{
  return trendExhaust;
}