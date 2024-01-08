#include <mcp2515.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

struct can_frame canMsg;

struct frameFlameSensor
{
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[3];
};

struct frameVoltageSensor
{
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[4];
    double temperature = 0;
    double voltage = 0;
};

byte spinner1[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
};

byte spinner2[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
};

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

MCP2515 mcp2515(10);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char spinnerChars[] = {1, '/', '-', 0};
unsigned long lastMessageTime = 0;
const unsigned long spinnerInterval = 400;     // Update spinner every 200 ms
const unsigned long stopSpinnerTimeout = 5000; // Stop spinner if no message for 5 seconds
static uint8_t spinnerIndex = 0;

void processFrameFlameSensor(const frameFlameSensor &sensorData)
{
    for (int i = 0; i < sensorData.can_dlc; ++i)
    {
        Serial.print(sensorData.data[i]);
        Serial.print(' ');
    }
    Serial.println();
}

void updateSpinner(char spinnerChar)
{
    lcd.setCursor(15, 1);
    lcd.print(spinnerChar);
}

void updateVoltageDisplay(const frameVoltageSensor &sensorData)
{
    lcd.setCursor(0, 0);
    lcd.print(sensorData.voltage);

    lcd.setCursor(0, 1);
    lcd.print(sensorData.temperature);
}

void processFrameVoltageSensor(frameVoltageSensor &sensorData)
{
    for (size_t i = 0; i < sizeof(sensorData.data); i++)
    {
        Serial.print("[");
        Serial.print(sensorData.data[i]);
        Serial.print("]");
    }
    Serial.println("");

    uint16_t reconstructedValue =
        (static_cast<uint8_t>(sensorData.data[0])) |
        (static_cast<uint8_t>(sensorData.data[1]) << 8);

    int16_t reconstructedTemperature =
        (static_cast<int8_t>(sensorData.data[2])) |
        (static_cast<int8_t>(sensorData.data[3]) << 8);

    double newTemperature = static_cast<double>(reconstructedTemperature) / 100;
    double newVoltage = static_cast<double>(reconstructedValue) / 100;

    if (newVoltage != sensorData.voltage)
    {
        sensorData.voltage = newVoltage;

        Serial.print("Received message with ID: 0x");
        Serial.println(sensorData.can_id, HEX);
        Serial.print("Voltage: ");
        Serial.println(sensorData.voltage);
        Serial.print("Temperature: ");
        Serial.println(sensorData.temperature);

        updateVoltageDisplay(sensorData);
        lastMessageTime = millis();  // Update last message time
    }
}

void copyCanMsgToStruct(const struct can_frame &canMsg, struct frameFlameSensor &flameSensorData)
{
    flameSensorData.can_id = canMsg.can_id;
    flameSensorData.can_dlc = canMsg.can_dlc;

    for (int i = 0; i < flameSensorData.can_dlc; ++i)
    {
        flameSensorData.data[i] = canMsg.data[i];
    }
}

void copyCanMsgToStruct(const struct can_frame &canMsg, struct frameVoltageSensor &voltageSensorData)
{
    voltageSensorData.can_id = canMsg.can_id;
    voltageSensorData.can_dlc = canMsg.can_dlc;

    for (int i = 0; i < voltageSensorData.can_dlc; ++i)
    {
        voltageSensorData.data[i] = canMsg.data[i];
    }
}

void setup()
{
    Serial.begin(9600);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print(" Diesel Heater");
    lcd.setCursor(0, 1);
    lcd.print("  initialized...");

    mcp2515.reset();
    mcp2515.setBitrate(CAN_125KBPS);
    mcp2515.setNormalMode();

    lcd.createChar(0, customBackslash);
    lcd.createChar(1, customVertical);

    delay(2000);
    lcd.clear();
}

void loop()
{
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)
    {
        if (canMsg.can_id == 0x100)
        {
            frameFlameSensor flameSensorData;

            if (canMsg.can_dlc == sizeof(flameSensorData.data))
            {
                copyCanMsgToStruct(canMsg, flameSensorData);
                processFrameFlameSensor(flameSensorData);
            }
            else
            {
                Serial.println("Error: Size mismatch in received 0x100 message");
            }
        }
        else if (canMsg.can_id == 0x101)
        {
            frameVoltageSensor voltageSensorData;

            if (canMsg.can_dlc == sizeof(voltageSensorData.data))
            {
                copyCanMsgToStruct(canMsg, voltageSensorData);
                processFrameVoltageSensor(voltageSensorData);
            }
            else
            {
                Serial.println("Error: Size mismatch in received 0x101 message");
            }
        }
    }

    unsigned long currentTime = millis();

    if (currentTime - lastMessageTime < stopSpinnerTimeout)
    {
        if (currentTime - lastMessageTime >= spinnerInterval)
        {
            updateSpinner(spinnerChars[spinnerIndex]);
            spinnerIndex = (spinnerIndex + 1) % (sizeof(spinnerChars) / sizeof(spinnerChars[0]));
            lastMessageTime = currentTime;
        }
    }
    else
    {
        // Stop spinner if no message for stopSpinnerTimeout milliseconds
        lcd.setCursor(15, 1);
        lcd.print(' '); // Clear the spinner character
    }
}
