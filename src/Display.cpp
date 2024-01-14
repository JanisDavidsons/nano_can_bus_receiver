#include "Display.h"

const char Display::spinnerChars[] = {1, '/', '-', 0};
const char Display::arrows[]       = {2, 3};
uint8_t Display::spinnerIndex      = 0;

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
  lcd.createChar(4, vertical),

  lcd.setCursor(0, 0);
  lcd.print(" Diesel Heater");
  lcd.setCursor(0, 1);
  lcd.print("  initialized...");

  delay(2000);
  lcd.clear();
}

void Display::updateVoltage(double voltage)
{
  lcd.setCursor(7,0);
  lcd.print((char)4);
  lcd.setCursor(0, 0);
  lcd.print(voltage);
  lcd.print("V");
}

void Display::updateTemperature(double temperature, CanBusReceiver::Trend trend)
{
  lcd.setCursor(7,1);
  lcd.print((char)4);
  lcd.setCursor(0, 1);
  lcd.print(temperature);

  if (temperature < 100.0 && trend == CanBusReceiver::TREND_EXHAUST_DECREASING)
  {
   lcd.setCursor(6, 1);
   lcd.print(" ");
   lcd.setCursor(5, 1);
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
    lcd.print(" ");
    break;
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
