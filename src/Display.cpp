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
  lcd.print(String(temperature, 0));
  printTrend(trend);
  lcd.setCursor(16, 1);
  lcd.print(String(temperature, 0));
  printTrend(trend);

  if (temperature < 100 && trend == CanBusReceiver::TREND_EXHAUST_DECREASING)
  {
    lcd.setCursor(3, 1);
    lcd.print(" ");
    lcd.setCursor(2, 1);

    lcd.setCursor(19, 1);
    lcd.print(" ");
    lcd.setCursor(18, 1);
  }
}

void Display::updateHeaterState(uint8_t state)
{
  if (holdOffTimerStarted && (millis() - displayStartTime <= holdOffScrollTime))
  {
    return;
  }
  holdOffTimerStarted = false;

  const char *message = stateTitleMap[state];
  int messageLength = strlen(message);

  if (state != previousState )
  {
    clearDisplay();
    previousState = state;
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
      holdOffTimerStarted = true;
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

        lcd.write(message[index]);
        charsWritten = index + 1;
      }

      // If the first part of the message has been displayed, start the hold off timer
      if (startCharIndex == 0)
      {
        holdOffTimerStarted = true;
        displayStartTime = millis();
      }

      startCharIndex++;
      lastUpdateTime = millis();
    }
  }
}

void Display::updateHeaterMode(uint8_t mode)
{
  if (mode != previousMode)
  {
    lcd.setCursor(16, 0);
    lcd.print("                ");
    lcd.setCursor(16, 0);
    lcd.print(operationMap[mode]);
    previousMode = mode;
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

void Display::scroll()
{
  // lcd.scrollDisplayLeft();
  if (!scrolledLeft)
  {
    for (size_t i = 0; i < 16; i++)
    {
      lcd.scrollDisplayLeft();
    }
    scrolledLeft = true;
  }
  else
  {
    for (size_t i = 0; i < 16; i++)
    {
      lcd.scrollDisplayRight();
    }
    scrolledLeft = false;
  }
}

void Display::returnHome()
{
  lcd.home();
  scrolledLeft = false;
}

void Display::printTrend(CanBusReceiver::Trend trend)
{
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