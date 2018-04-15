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

//add for later
const static char ch_Left     = 127;
const static char ch_Right    = 126;
const static char ch_Dot      = -91;
const static char ch_Degree   = -33;

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
char varBuffer[6];

struct ButtonControl
{
    unsigned long firstTime = 0;
    long secs_held;
    long prev_secs_held;
    boolean current;
    boolean previous = true;
} LCDButton;

int setTemp = 32;
boolean isTempSet = false;

float nowTemp = 0.0f;

void loop()
{
    if (!isTempSet)
    {
        sprintf(displayBuffer, "Set to:%-5d%cC ", setTemp, ch_Degree);
        lcd.setCursor(0, 0);
        printLCDBuffer();
        controlValue(&setTemp, &isTempSet);
    }
    if (isTempSet)
    {
        sprintf(displayBuffer, "Set at:%-5d%cC ", setTemp, ch_Degree);
        lcd.setCursor(0, 0);
        printLCDBuffer();
        tempSensor.requestTemperatures();
        nowTemp = tempSensor.getTempCByIndex(0);
        dtostrf(nowTemp, -5, 1, varBuffer);
        sprintf(displayBuffer, "Now is:%s%cC ", varBuffer, ch_Degree);
        lcd.setCursor(0, 1);
        printLCDBuffer();
    }
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

void printLCDBuffer()
{
    lcd.print(displayBuffer);
}

void modValue(int *ptr)
{
    if (readLCDButton() == LCD_BTN_LEFT)
    {
        (*ptr)--;
    }
    if (readLCDButton() == LCD_BTN_RIGHT)
    {
        (*ptr)++;
    }
}

void modBoolean(boolean *ptr)
{
    if (readLCDButton() == LCD_BTN_SELECT)
    {
        *ptr = !(*ptr);
    }
}

void controlValue(int *value, boolean *isSet)
{
    LCDButton.current = (readLCDButton() == LCD_BTN_NONE);
    if (LCDButton.current == false && LCDButton.previous == true && (millis() - LCDButton.firstTime) > 50)
    {
        LCDButton.firstTime = millis();
        modValue(value);
        modBoolean(isSet);
    }

    LCDButton.secs_held = (millis() - LCDButton.firstTime) / 1000;

    if (LCDButton.current == false && LCDButton.secs_held > LCDButton.prev_secs_held)
    {
        modValue(value);
        modBoolean(isSet);
    }

    LCDButton.previous = LCDButton.current;
    LCDButton.prev_secs_held = LCDButton.secs_held;
}



