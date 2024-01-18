#include "CanBusReceiver.h"

CanBusReceiver::CanBusReceiver(MCP2515 &mcp2515)
    : mcp2515(mcp2515),
      trendExhaust(TREND_EXHAUST_NOT_CHANGING),
      voltageChanged(true)
{
}

template <typename T>
void CanBusReceiver::copyCanMsgToStruct(const struct can_frame &canMsg, T &frame)
{
  frame.can_id = canMsg.can_id;
  frame.can_dlc = canMsg.can_dlc;

  for (int i = 0; i < frame.can_dlc; ++i)
  {
    frame.data[i] = canMsg.data[i];
  }
}

void CanBusReceiver::processFrameVoltageSensor(frameVoltageSensor &voltageData)
{
  uint16_t reconstructedValue =
      (static_cast<uint8_t>(voltageData.data[0])) |
      (static_cast<uint8_t>(voltageData.data[1]) << 8);

  double newVoltage = static_cast<double>(reconstructedValue) / 100;

  if (newVoltage != voltageData.voltage)
  {
    voltageData.voltage = newVoltage;
    voltageChanged = true;
    lastMessageTime = millis();
  }
}

void CanBusReceiver::processFrameFlameSensor(frameFlameSensor &flameData)
{
  int16_t reconstructedTemperature =
      (flameData.data[0]) |
      (flameData.data[1] << 8);

  double newTemperature = round(static_cast<double>(reconstructedTemperature) / 10);

  // tempData.surface = round(static_cast<double>(reconstructedSurfaceTmp) / 10);


  flameData.value = newTemperature;
  flameData.isIncreasing = static_cast<bool>(flameData.data[2]);
  flameData.isDecreasing = static_cast<bool>(flameData.data[3]);

  setTrend(flameData);
}

void CanBusReceiver::processFrameHeaterState(frameHeaterState &stateData)
{
  stateData.state = stateData.data[0];
}

void CanBusReceiver::processFrameHeaterTemperature(frameTemperature &tempData)
{
  int16_t reconstructedCoolantTmp =
      (tempData.data[0]) |
      (tempData.data[1] << 8);

  int16_t reconstructedSurfaceTmp =
      (tempData.data[2]) |
      (tempData.data[3] << 8);

  tempData.coolant = 25.2;
  tempData.coolant = static_cast<double>(reconstructedCoolantTmp) / 10;
  tempData.surface = static_cast<double>(reconstructedSurfaceTmp) / 10;
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
    else if (canMsg.can_id == 0x102)
    {
      if (canMsg.can_dlc == sizeof(heaterState.data))
      {
        copyCanMsgToStruct(canMsg, heaterState);
        processFrameHeaterState(heaterState);
      }
      else
      {
        Serial.println("Error: Size mismatch in received 0x102 message");
      }
    }
    else if (canMsg.can_id == 0x103)
    {
      if (canMsg.can_dlc == sizeof(heaterTemperature.data))
      {
        copyCanMsgToStruct(canMsg, heaterTemperature);
        processFrameHeaterTemperature(heaterTemperature);
      }
      else
      {
        Serial.println("Error: Size mismatch in received 0x103 message");
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

uint8_t CanBusReceiver::getHeaterStateIndex()
{
  return heaterState.state;
}

double CanBusReceiver::getCoolantTmp()
{
  return heaterTemperature.coolant;
}

double CanBusReceiver::getSurfaceTmp()
{
  return heaterTemperature.surface;
}