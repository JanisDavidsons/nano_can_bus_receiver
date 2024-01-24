#include <Arduino.h>
#include <mcp2515.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <arduino-timer.h>
#include <time.h>

#include "CanBusReceiver.h"
#include "Display.h"
#include "RoomTemperature.h"

extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;

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

int freeMemory()
{
    int free_memory;
    if ((int)__brkval == 0)
        free_memory = ((int)&free_memory) - ((int)&__heap_start);
    else
        free_memory = ((int)&free_memory) - ((int)__brkval);
    return free_memory;
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
    lcd.updateHeaterMode(canbus.getHeaterModeIndex());

    handleButton();
}

void runTimeCallback()
{
    timer.every(
        15000,
        [](void *) -> bool
        {
            // Print room temperature every 5 seconds, will be sent to the heater later on.
            
            /**
             * Im running out of the memory if I uncoment this. 
             * Probably have to refactor the code, to make everything more
             * efficient.
            */
            // Serial.print("Room temperature: ");
            // Serial.println(temperature.readTemperature());
            return true;
        });

    timer.every(
        500,
        [](void *) -> bool
        {
            Serial.print("Free memory: ");
            Serial.println(freeMemory());
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
                lcd.scroll();
            }
        }
    }
}
