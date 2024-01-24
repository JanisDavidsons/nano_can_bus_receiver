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
    char currentMessage[15]; // Maximum length of the message
    bool messageWritten = false;
    bool scrolledLeft = false;
    bool holdOffTimerStarted = false;
    uint64_t scrollSpeed = 500;
    unsigned long lastUpdateTime = 0;
    unsigned long displayStartTime = 0;
    // unsigned long scroollEndTime = 0;
    int startCharIndex = 0;
    int charsWritten = 0;
    int displayLength = 9;             // Number of characters to display at a time
    uint16_t holdOffScrollTime = 2000; // Time to hold the message in milliseconds

    uint8_t previousState = 255;
    uint8_t previousMode = 255;

    const char *stateTitleMap[6] = {
        "off",
        "sleep",
        "starting",
        "running",
        "shutting down",
        "restarting"};

    const char *operationMap[12] = {
        "Switch off",
        "Low voltage",
        "Pre-start",
        "Venting",
        "Priming",
        "Igniting",
        "Active",
        "Idle",
        "Eco",
        "Cooling < 150",
        "Startup failure",
        "Flame out"};

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

    byte noChange[8] = {
        B00100,
        B01110,
        B10101,
        B00100,
        B10101,
        B01110,
        B00100,
        B00000};

    void printTrend(CanBusReceiver::Trend trend);

public:
    Display(LiquidCrystal_I2C &lcd);
    void initialize();
    void updateVoltage(double voltage);
    void updateFlameTmp(double temperature, CanBusReceiver::Trend trend);
    void updateHeaterState(uint8_t state);
    void updateHeaterMode(uint8_t state);
    void updateHeaterTemperature(double coolant, double surface);
    void updateSpinner();
    void clearDisplay();
    void scroll();
    void returnHome();
};

#endif