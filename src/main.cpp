#include <Arduino.h>
#include <mcp2515.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <arduino-timer.h>
#include <time.h>

#include "CanBusReceiver.h"
#include "Display.h"
#include "RoomTemperature.h"

uint8_t buttonPin = 4;
int buttonState = 0;

MCP2515 mcp2515(10);
CanBusReceiver canbus(mcp2515);
DHT_Unified dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C liquidCrystal_I2C(0x27, 16, 2);
Display lcd(liquidCrystal_I2C);
RoomTemperature temperature(dht);

/* timed events */
auto timer = timer_create_default();

const unsigned long stopSpinnerTimeout = 5000; // Stop spinner if no CAN BUS message for 5 seconds
const int debounceDelay = 50;                 
unsigned long lastDebounceTime = 0;
int lastButtonState = HIGH;

void runTimeCallback();
void handleButton();

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);

    Serial.begin(9600);

    lcd.initialize();
    temperature.initilize();

    mcp2515.reset();
    mcp2515.setBitrate(CAN_125KBPS);
    mcp2515.setNormalMode();
    runTimeCallback();
}

void loop()
{
    timer.tick();
    unsigned long currentTime = millis();

    canbus.checkMessage();

    if (canbus.voltageChanged)
    {
        lcd.updateVoltage(canbus.getVoltage());
        canbus.voltageChanged = false;
    }

    lcd.updateFlameTmp(canbus.getExhaustTemp(), canbus.getTExhaustTrend());
    lcd.updateHeaterTemperature(canbus.getCoolantTmp(), canbus.getSurfaceTmp());

    if (currentTime - canbus.getLatMessageTime() < stopSpinnerTimeout)
    {
        lcd.updateSpinner();
    }

    lcd.updateHeaterState(canbus.getHeaterStateIndex());

    handleButton();
}

void runTimeCallback()
{
    timer.every(
        15000,
        [](void *) -> bool
        {
            // Print room temperature every 5 seconds, will be sent to the heater later on.
            Serial.print("Room temperature: ");
            Serial.println(temperature.readTemperature());

            lcd.returnHome();

            return true;
        });
}

void handleButton()
{
    int reading = digitalRead(buttonPin);

    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
        lastButtonState = reading;
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (reading != buttonState)
        {
            buttonState = reading;

            if (buttonState == LOW)
            {
                lcd.scrollRight();
            }
        }
    }
}
