#include <Arduino.h>
#include <mcp2515.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "CanBusReceiver.h"
#include "Display.h"
#include "RoomTemperature.h"

MCP2515 mcp2515(10);
CanBusReceiver canbus(mcp2515);
DHT_Unified dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C liquidCrystal_I2C(0x27, 16, 2);
Display lcd(liquidCrystal_I2C);
RoomTemperature temperature(dht);

const unsigned long stopSpinnerTimeout = 5000; // Stop spinner if no CAN BUS message for 5 seconds

void setup()
{
    Serial.begin(9600);

    lcd.initialize();
    temperature.initilize();

    mcp2515.reset();
    mcp2515.setBitrate(CAN_125KBPS);
    mcp2515.setNormalMode();
}

void loop()
{
    unsigned long currentTime = millis();

    canbus.checkMessage();

    if (canbus.voltageChanged)
    {
        lcd.updateVoltage(canbus.getVoltage());
        canbus.voltageChanged = false;
    }

    lcd.updateTemperature(canbus.getExhaustTemp(), canbus.getTExhaustTrend());

    if (currentTime - canbus.getLatMessageTime() < stopSpinnerTimeout)
    {
        lcd.updateSpinner();
    }

    // Print room temperature every 5 seconds, will be sent to the heate late on.
    static unsigned long lastTemperaturePrintTime = 0;
    const unsigned long temperaturePrintInterval = 5000;

    if (currentTime - lastTemperaturePrintTime >= temperaturePrintInterval)
    {
        Serial.print("Room temperature: ");
        Serial.println(temperature.readTemperature());

        lastTemperaturePrintTime = currentTime;
    }
}
