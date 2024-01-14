#ifndef LCD_DISPLAY
#define LCD_DISPLAY

#include <LiquidCrystal_I2C.h>
#include "CanBusReceiver.h"

class Display
{
private:
    LiquidCrystal_I2C &lcd;
    unsigned long spinnerTime = 0;
    const unsigned long spinnerInterval = 150; // Update spinner every 200 ms
    static uint8_t spinnerIndex;
    static const char spinnerChars[];
    static const char arrows[];

    byte customBackslash[8] = {
        B00000,
        B10000,
        B01000,
        B00100,
        B00010,
        B00001,
        B00000,
        B00000,
    };

    byte customVertical[8] = {
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B00000,
        B00000,
    };

    byte increase[8] = {
        B00100,
        B01110,
        B10101,
        B00100,
        B00100,
        B00100,
        B00100,
        B00000};

    byte decrease[8] = {
        B00100,
        B00100,
        B00100,
        B00100,
        B10101,
        B01110,
        B00100,
        B00000};

    byte vertical[8] = {
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100};

public:
    Display(LiquidCrystal_I2C &lcd);
    void initialize();
    void updateVoltage(double voltage);
    void updateTemperature(double temperature, CanBusReceiver::Trend trend);
    void updateSpinner();
};

#endif