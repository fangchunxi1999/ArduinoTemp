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

#define LED_LIGHT   50
#define tonePin     48

//add for later
const static char ch_Left     = 127;
const static char ch_Right    = 126;
const static char ch_Dot      = -91;
const static char ch_Degree   = -33;
const static char ch_Block    = -1;

#define maxMode 2

//for debug
#define debugButton false
#define debugSerialPrint false

void setup()
{
    lcd.begin(LCD_COLS, LCD_ROWS);
    printDisplayln("Hello World!!", 0);

    tempSensor.begin();
    tempSensor.setResolution(12);

    pinMode(LED_LIGHT, OUTPUT);
    digitalWrite(LED_LIGHT, HIGH);

    #if (debugSerialPrint)
    Serial.begin(115200);
    #endif

    delay(1000);
    digitalWrite(LED_LIGHT, LOW);
    lcd.clear();
}

char displayBuffer[17] = "";
char varBuffer[6];

struct ButtonControl
{
    unsigned long firstTime = 0;
    int secs_held;
    int prev_secs_held;
    boolean current;
    boolean previous = true;
} buttonValue;

int mode   = 0;

int setTemp     = 32;
float nowTemp   = 0.0f;

boolean isTempSet   = false;
boolean isModeSet   = false;
boolean isAlarmSet  = false;
boolean isPlayTone  = false;

void loop()
{
    if (!isModeSet)
    {
        menuSetMode();
        return;
    }
    else
    {
        switch (mode % 2)
        {
            case 1: menuGetTemp();      return;
        }
    }

    if (!isTempSet)
    {
        menuSetTemp();
        return;
    }
    if (isTempSet)
    {
        menuCheckGetTemp();
    }
}

//debug function
#if (debugButton)
void printbuttonValue(byte key)
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
#endif

//debug function


//LCD Display function
void printDisplayTempCByIndex(int index)
{
    nowTemp = getTempCByIndex(index);
    dtostrf(nowTemp, -5, 1, varBuffer);
    printDisplayTemp("Now is:", varBuffer, 1);
}

void printDisplayHead(char *str)
{
    printDisplayln(str, 0);
}

void printDisplayln(char *str, byte row)
{
    sprintf(displayBuffer, "%s", str);
    printDisplayBuffer(row);
}

void printDisplayTemp(char *str, int var, byte row)
{
    sprintf(displayBuffer, "%s%-5d%cC  " , str, var, ch_Degree);
    printDisplayBuffer(row);
}

void printDisplayTemp(char *str0, char *str1, byte row)
{
    sprintf(displayBuffer, "%s%s%cC  " , str0, str1, ch_Degree);
    printDisplayBuffer(row);
}

void printDisplayBuffer(byte row)
{
    lcd.setCursor(0, row);
    lcd.print(displayBuffer);
}
//LCD Display function


//Value function


//condition control
void controlValue(int *value, boolean *isSet, int modClick, int modHold)
{
    byte button = getButton();
    buttonValue.current = (button == LCD_BTN_NONE);
    if (buttonValue.current == false && buttonValue.previous == true && (millis() - buttonValue.firstTime) > 50)
    {
        buttonValue.firstTime = millis();
        #if (debugButton)
        lcd.setCursor(0, 1);
        printbuttonValue(button);
        #endif
        switch (button)
        {
            case LCD_BTN_LEFT:  (*value) -= modClick; break;
            case LCD_BTN_RIGHT: (*value) += modClick; break;
            case LCD_BTN_SELECT: *isSet = !(*isSet); break;
        }
    }

    buttonValue.secs_held = (millis() - buttonValue.firstTime) / 1000;

    if (buttonValue.current == false && buttonValue.secs_held > buttonValue.prev_secs_held)
    {
        switch (button)
        {
            case LCD_BTN_LEFT:  (*value) -= modHold; break;
            case LCD_BTN_RIGHT: (*value) += modHold; break;
        }
    }

    buttonValue.previous = buttonValue.current;
    buttonValue.prev_secs_held = buttonValue.secs_held;
}

boolean controlCancel(int holdSecs)
{
    byte button = getButton();
    buttonValue.current = (button == LCD_BTN_NONE);
    if (buttonValue.current == false && buttonValue.previous == true && (millis() - buttonValue.firstTime) > 50)
    {
        buttonValue.firstTime = millis();
        if (button == LCD_BTN_SELECT)
        {
            lcd.clear();
            printDisplayHead("Hold to cancel..");
        }
    }

    buttonValue.secs_held = (millis() - buttonValue.firstTime) / 1000;

    if (buttonValue.current == false && buttonValue.secs_held > buttonValue.prev_secs_held)
    {
        if (button == LCD_BTN_SELECT)
        {
            sprintf(displayBuffer, "%3d/%2d Secs", buttonValue.secs_held, holdSecs);
            printDisplayBuffer(1);
        }
        if (buttonValue.secs_held >= holdSecs)
        {
            lcd.clear();
            noTone(tonePin);
            digitalWrite(LED_LIGHT, LOW);
            isModeSet = false;
            isTempSet = false;
            isAlarmSet = false;
        }
    }

    buttonValue.previous = buttonValue.current;
    buttonValue.prev_secs_held = buttonValue.secs_held;
    return !buttonValue.current;
}
//condition control


//UI (WIP)
void menuSetMode()
{
    controlValue(&mode, &isModeSet, 1, 1);
    int i = mode % maxMode;
    sprintf(displayBuffer, "Set Mode:%2d", i + 1);
    printDisplayBuffer(0);
    switch (i)
    {
        case 0:
            printDisplayln("Alarm Temp", 1);
            mode = 0;
            break;
        case 1:
            printDisplayln("Read Only ", 1);
            mode = 1;
            break;

        default:
            printDisplayln("Err", 1);
            mode = maxMode;
            break;
    }
}

void menuSetTemp()
{
    printDisplayTemp("Set to:", setTemp, 0);
    controlValue(&setTemp, &isTempSet, 1, 5);
}

void menuGetTemp()
{
    if (controlCancel(10))
    {
        return;
    }
    printDisplayHead("Display current");
    printDisplayTempCByIndex(0);
}

void menuCheckGetTemp()
{
    if (!isAlarmSet)
    {
        if (controlCancel(10))
        {
            return;
        }
    }
    
    if (nowTemp >= setTemp || isAlarmSet)
    {
        isAlarmSet = true;
        menuTempAlarm(0);
        return;
    }

    printDisplayTemp("Set to:", setTemp, 0);
    printDisplayTempCByIndex(0);
}

void menuTempAlarm(int index)
{
    if (controlCancel(3))
    {
        return;
    }
    digitalWrite(LED_LIGHT, HIGH);
    printDisplayHead("Set Alarm Out...");
    printDisplayTempCByIndex(index);
    if (!isPlayTone)
    {
        playTone();
    }
}
//UI


//Alarm play (WIP)
void playTone()
{
    isPlayTone = true;
    tone(tonePin, 1000);
}
//Alarm play (WIP)


//misc
byte getButton()
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

float getTempCByIndex(int index)
{
    tempSensor.requestTemperatures();
    return tempSensor.getTempCByIndex(index);
}
//misc
