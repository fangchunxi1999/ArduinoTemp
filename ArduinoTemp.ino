//Digital Temperature Prob Library
#include <OneWire.h>
#include <DallasTemperature.h>

//LCD Library
#include <LiquidCrystal.h>

//for LCD Screen
#define LCD_RS      8
#define LCD_ENABLE  9
#define LCD_D4      4
#define LCD_D5      5
#define LCD_D6      6
#define LCD_D7      7
#define LCD_COLS    16
#define LCD_ROWS    2
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//for Button value
#define LCD_BUTTON      A0
#define LCD_BTN_NONE    0
#define LCD_BTN_SELECT  1
#define LCD_BTN_UP      2
#define LCD_BTN_DOWN    3
#define LCD_BTN_LEFT    4
#define LCD_BTN_RIGHT   5

//for Digital Temperature Prob
#define TEMP_READ   A8
OneWire oneWire(TEMP_READ);
DallasTemperature tempSensor(&oneWire);

void setup()
{
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setCursor(0, 0);
    lcd.print("Hello, World!");

    tempSensor.begin();
    tempSensor.setResolution(12);

    Serial.begin(115200);

    delay(1000);
    lcd.clear();
}

char displayBuffer[17] = "";
char varBuffer[8];

void loop()
{
    lcd.setCursor(0, 0);
    lcd.print("Current Temp:");
    lcd.setCursor(0, 1);
    tempSensor.requestTemperatures();
    float temp = tempSensor.getTempCByIndex(0);
    dtostrf(temp, 7, 2, varBuffer);
    sprintf(displayBuffer, "%s C", varBuffer);
    lcd.print(displayBuffer);
}

int readLCDButton()
{
    int key = analogRead(LCD_BUTTON);
    if (key > 1000) return LCD_BTN_NONE;

    if (key < 50)   return LCD_BTN_RIGHT;
    if (key < 150)  return LCD_BTN_UP;
    if (key < 300)  return LCD_BTN_DOWN;
    if (key < 500)  return LCD_BTN_LEFT;
    if (key < 750)  return LCD_BTN_SELECT;

    return -1;
}

void printLCDButton(int key)
{
    switch (key)
    {
        case LCD_BTN_NONE:
        {
            lcd.print("NONE  ");
            break;
        }
        case LCD_BTN_SELECT:
        {
            lcd.print("SELECT");
            break;
        }
        case LCD_BTN_UP:
        {
            lcd.print("UP    ");
            break;
        }
        case LCD_BTN_DOWN:
        {
            lcd.print("DOWN  ");
            break;
        }
        case LCD_BTN_LEFT:
        {
            lcd.print("LEFT  ");
            break;
        }
        case LCD_BTN_RIGHT:
        {
            lcd.print("RIGHT ");
            break;
        }
    }
}
