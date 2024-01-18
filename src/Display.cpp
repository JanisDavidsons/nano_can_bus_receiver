#include "Display.h"

const char Display::spinnerChars[] = {1, '/', '-', 0};
const char Display::arrows[] = {2, 3, 4};
uint8_t Display::spinnerIndex = 0;

Display::Display(LiquidCrystal_I2C &lcd)
    : lcd(lcd)
{
}

void Display::initialize()
{
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, customBackslash);
  lcd.createChar(1, customVertical);
  lcd.createChar(2, increase);
  lcd.createChar(3, decrease);
  lcd.createChar(4, noChange);

  lcd.setCursor(0, 0);
  lcd.print(" Diesel Heater");
  lcd.setCursor(0, 1);
  lcd.print("  initialized...");

  delay(2000);
  lcd.clear();
}

void Display::updateFlameTmp(double temperature, CanBusReceiver::Trend trend)
{
  lcd.setCursor(0, 1);
  lcd.print(String(temperature,0));

  if (temperature < 100 && trend == CanBusReceiver::TREND_EXHAUST_DECREASING)
  {
    lcd.setCursor(4, 1);
    lcd.print(" ");
    lcd.setCursor(3, 1);
  }

  switch (trend)
  {
  case CanBusReceiver::TREND_EXHAUST_RISING:
    lcd.print(arrows[0]);
    break;
  case CanBusReceiver::TREND_EXHAUST_DECREASING:
    lcd.print(arrows[1]);
    break;
  default:
    lcd.print(arrows[2]);
    break;
  }
}

void Display::updateHeaterState(uint8_t state)
{
  const char *message = stateTitleMap[state];
  int messageLength = strlen(message);

  // Check if the new message is different from the previous one
  if (strcmp(message, previousMessage) != 0)
  {
    // Clear the display only when the message has changed
    clearDisplay();

    // Copy the new message to the previousMessage variable
    strncpy(previousMessage, message, sizeof(previousMessage) - 1);
    previousMessage[sizeof(previousMessage) - 1] = '\0';
  }

  if (messageLength < 9)
  {
    // If the message is shorter than 9 characters, display it without scrolling
    lcd.setCursor(7, 0);
    lcd.print(message);
    return;
  }

  if (millis() - lastUpdateTime >= scrollSpeed)
  {
    lcd.setCursor(7, 0);
    // If the entire message has been displayed, start the hold off timer
    if (charsWritten >= messageLength)
    {
      if (millis() - displayStartTime >= holdOffScrollTime)
      {
        displayStartTime = millis();
        startCharIndex = 0;
        charsWritten = 0;
      }
    }
    else
    {
      for (int j = 0; j < displayLength; ++j)
      {
        if (charsWritten >= messageLength)
        {
          charsWritten = 0;
          startCharIndex = 0;
          clearDisplay();
          lcd.setCursor(7, 0);
        }

        int index = (startCharIndex + j) % messageLength;
        Serial.println(index);
        lcd.write(message[index]);
        charsWritten = index + 1;
      }

      // Reset the displayStartTime after the first loop
      if (startCharIndex == 1)
      {
        displayStartTime = millis();
      }

      startCharIndex++;
      lastUpdateTime = millis();
    }
  }
}

void Display::updateSpinner()
{
  unsigned long currentTime = millis();

  if (currentTime - spinnerTime >= spinnerInterval)
  {
    lcd.setCursor(15, 1);
    lcd.print(spinnerChars[spinnerIndex]);
    spinnerIndex = (spinnerIndex + 1) % (sizeof(spinnerChars) / sizeof(spinnerChars[0]));

    spinnerTime = currentTime;
  }
}

void Display::updateVoltage(double voltage)
{
  lcd.setCursor(0, 0);
  lcd.print(voltage);
  lcd.print("V");
}

void Display::updateHeaterTemperature(double coolant, double surface)
{
  lcd.setCursor(5, 1);
  lcd.print(String(coolant, 1));
  lcd.setCursor(10, 1);
  lcd.print(String(surface, 1));
}

void Display::clearDisplay()
{
  lcd.setCursor(7, 0);
  lcd.print("         ");
}

void Display::scrollRight()
{
  if (!scrolledRight)
  {
    for (size_t i = 0; i < 16; i++)
    {
      lcd.scrollDisplayRight();
    }
    scrolledRight = true;
  }
  else
  {
    for (size_t i = 0; i < 16; i++)
    {
      lcd.scrollDisplayLeft();
    }
    scrolledRight = false;
  }
}

void Display::returnHome()
{
  lcd.home();
  scrolledRight = false;
}

void Display::shiftRight()
{
}