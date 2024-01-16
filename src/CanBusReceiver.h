#ifndef CAN_BUS_RECEIVER
#define CAN_BUS_RECEIVER

#include <Arduino.h>
#include <mcp2515.h>

class CanBusReceiver
{
public:
  enum Trend
  {
    TREND_EXHAUST_NOT_CHANGING,
    TREND_EXHAUST_RISING,
    TREND_EXHAUST_DECREASING
  };

private:
  MCP2515 &mcp2515;
  unsigned long lastMessageTime = 0;

  struct can_frame canMsg;

  Trend trendExhaust;

  struct frameFlameSensor
  {
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[4];
    double value = 0;
    bool isIncreasing = false;
    bool isDecreasing = false;
  } exhaustTemperature;

  struct frameVoltageSensor
  {
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[2];
    double voltage = 0;
  } voltageSensor;

  struct frameHeaterState
  {
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[1];
    uint8_t state = 0;
  } heaterState;

  struct frameTemperature
  {
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[4];
    double coolant = 0;
    double surface = 0;
  } heaterTemperature;

  void setTrend(frameFlameSensor &sensor);

public:
  bool voltageChanged;
  bool temperatureChanged;

  CanBusReceiver(MCP2515 &mcp2515);
  const struct can_frame &getCanMsg() const;
  double getTemperature() const;
  double getExhaustTemp() const;
  double getVoltage() const;
  uint8_t getHeaterStateIndex();
  void checkMessage();
  unsigned long getLatMessageTime();
  bool isExhaustTempIncreasing();
  bool isExhaustTempDecreasing();
  Trend getTExhaustTrend();
  double getCoolantTmp();
  double getSurfaceTmp();

  template <typename T>
  void copyCanMsgToStruct(const struct can_frame &canMsg, T &frame);

  void processFrameVoltageSensor(frameVoltageSensor &voltageData);
  void processFrameFlameSensor(frameFlameSensor &flameData);
  void processFrameHeaterState(frameHeaterState &stateData);
  void processFrameHeaterTemperature(frameTemperature &temperatureData);
};

#endif
