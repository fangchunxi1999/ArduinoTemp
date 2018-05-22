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
#define CON_BUTTON      A0

#define CON_BTN_NONE    0
#define CON_BTN_SELECT  1
#define CON_BTN_UP      2
#define CON_BTN_DOWN    3
#define CON_BTN_LEFT    4
#define CON_BTN_RIGHT   5

#define CON_VAR_NONE    1000
#define CON_VAR_SELECR  750
#define CON_VAR_UP      150
#define CON_VAR_DOWN    300
#define CON_VAR_LEFT    500
#define CON_VAR_RIGHT   50

//for Digital Temperature Prob
#define TEMP_READ   A7
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

#define maxMode 4

//for debug
#define debugButton false
#define debugSerialPrint false

void setup()
{
    lcd.begin(LCD_COLS, LCD_ROWS);
    printDisplayln("Welcome!!", 0);

    pinMode(LED_LIGHT, OUTPUT);
    digitalWrite(LED_LIGHT, HIGH);

    tone(tonePin, 1000);

    tempSensor.begin();
    tempSensor.setResolution(12);

    #if (debugSerialPrint)
    Serial.begin(115200);
    #endif

    delay(1000);
    noTone(tonePin);
    digitalWrite(LED_LIGHT, LOW);
    lcd.clear();
}

char displayBuffer[LCD_COLS + 1] = "";
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

int setTemp     = 50;
float nowTemp   = 0.0f;

char greater = '>';
char symB = 'C';

boolean isTempSet   = false;
boolean isModeSet   = false;
boolean isAlarmSet  = false;
boolean isPlayTone  = false;
boolean isUseF      = false;
int isCheckGreater = 1;

#if (debugSerialPrint)
unsigned long timeDebug = 0;
#endif

void loop()
{
    #if (debugSerialPrint)
    Serial.print("Loop time:");
    Serial.println(millis() - timeDebug);
    timeDebug = millis();
    #endif

    if (!isModeSet)
    {
        menuSetMode();
        return;
    }
    else
    {
        switch (mode % 2)
        {
            case 0: 
                if (!isTempSet)
                {
                    menuSetTemp();
                    return;
                }
                menuCheckGetTemp();
                return;
            
            case 1: menuGetTemp();      return;
        }
    }
}

//debug function
#if (debugButton)
void printbuttonValue(byte key)
{
    switch (key)
    {
        case CON_BTN_NONE:
        {
            lcd.print("NONE  ");
            break;
        }
        case CON_BTN_SELECT:
        {
            lcd.print("SELECT");
            break;
        }
        case CON_BTN_UP:
        {
            lcd.print("UP    ");
            break;
        }
        case CON_BTN_DOWN:
        {
            lcd.print("DOWN  ");
            break;
        }
        case CON_BTN_LEFT:
        {
            lcd.print("LEFT  ");
            break;
        }
        case CON_BTN_RIGHT:
        {
            lcd.print("RIGHT ");
            break;
        }
    }
}
#endif

//debug function


//LCD Display function
void printDisplayTempByIndex(int index)
{
    nowTemp = getTempByIndex(index);
    dtostrf(nowTemp, -5, 1, varBuffer);
    printDisplayTemp("Now:", varBuffer, symB, 1);
}

void printDisplayln(char *str, byte row)
{
    sprintf(displayBuffer, "%s", str);
    printDisplayBuffer(row);
}


void printDisplayTemp(char *str0, char *str1,char symbol, byte row)
{
    sprintf(displayBuffer, "%s%s%c%c" , str0, str1, ch_Degree, symbol);
    printDisplayBuffer(row);
}

void printDisplayBuffer(byte row)
{
    lcd.setCursor(0, row);
    char disBuff[LCD_COLS + 1] = "";
    sprintf(disBuff, "%-16s", displayBuffer);
    #if (debugSerialPrint)
    Serial.println(disBuff);
    #endif
    lcd.print(disBuff);
}
//LCD Display function


//Value function


//condition control
void modValue(int *value, int mod)
{
    (*value) += mod;
}

void modValue(boolean *value)
{
    (*value) = !(*value);
}

void controlValue(int *valueLR, int *valueUD, boolean *isSet, int modClick, int modHold)
{
    byte button = getButton();
    #if (debugSerialPrint)
    Serial.print("1:");
    Serial.println(button);
    #endif
    buttonValue.current = (button == CON_BTN_NONE);
    if (buttonValue.current == false && buttonValue.previous == true && (millis() - buttonValue.firstTime) > 50)
    {
        buttonValue.firstTime = millis();
        #if (debugSerialPrint)
        Serial.print("2:");
        Serial.println(button);
        #endif
        if (valueLR != NULL)
        {
            switch (button)
            {
                case CON_BTN_LEFT:      modValue(valueLR, -modClick); break;
                case CON_BTN_RIGHT:     modValue(valueLR, modClick); break;
            }
        }
        if (valueUD != NULL)
        {
            switch (button)
            {
                case CON_BTN_DOWN:      modValue(valueUD, -modClick); break;
                case CON_BTN_UP:        modValue(valueUD, modClick); break;
            }
        }
        if (isSet != NULL)
        {
            if (button == CON_BTN_SELECT)
            {
                modValue(isSet);
            }
        }
    }

    buttonValue.secs_held = (millis() - buttonValue.firstTime) / 1000;

    if (buttonValue.current == false && buttonValue.secs_held > buttonValue.prev_secs_held)
    {
        if (valueLR != NULL)
        {
            switch (button)
            {
                case CON_BTN_LEFT:      modValue(valueLR, -modHold); break;
                case CON_BTN_RIGHT:     modValue(valueLR, modHold); break;
            }
        }
        if (valueUD != NULL)
        {
            switch (button)
            {
                case CON_BTN_DOWN:      modValue(valueUD, -modHold); break;
                case CON_BTN_UP:        modValue(valueUD, modHold); break;
            }
        }
    }

    buttonValue.previous = buttonValue.current;
    buttonValue.prev_secs_held = buttonValue.secs_held;
}

boolean controlCancel(int holdSecs)
{
    byte button = getButton();
    buttonValue.current = (button == CON_BTN_NONE);
    if (buttonValue.current == false && buttonValue.previous == true && (millis() - buttonValue.firstTime) > 50)
    {
        buttonValue.firstTime = millis();
        if (button == CON_BTN_SELECT)
        {
            lcd.clear();
            printDisplayln("Hold 2 cancel", 0);
        }
    }

    buttonValue.secs_held = (millis() - buttonValue.firstTime) / 1000;

    if (buttonValue.current == false && buttonValue.secs_held > buttonValue.prev_secs_held)
    {
        if (button == CON_BTN_SELECT)
        {
            sprintf(displayBuffer, "%3d/%2d Secs", buttonValue.secs_held, holdSecs);
            printDisplayBuffer(1);
        }
        if (buttonValue.secs_held >= holdSecs)
        {
            lcd.clear();
            noTone(tonePin);
            digitalWrite(LED_LIGHT, LOW);
            isTempSet   = false;
            isModeSet   = false;
            isAlarmSet  = false;
            isPlayTone  = false;
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
    controlValue(&mode, NULL, &isModeSet, 1, 1);
    int i = mode % maxMode;
    sprintf(displayBuffer, "Set:%2d", i + 1);
    printDisplayBuffer(0);
    switch (i)
    {
        case 0:
            printDisplayln("Alarm Temp C", 1);
            symB = 'C';
            isUseF = false;
            mode = 0;
            break;
        case 1:
            printDisplayln("Read Only C", 1);
            symB = 'C';
            isUseF = false;
            mode = 1;
            break;
        case 2:
            printDisplayln("Alarm Temp F", 1);
            symB = 'F';
            isUseF = true;
            mode = 2;
            break;
        case 3:
            printDisplayln("Read Only F", 1);
            symB = 'F';
            isUseF = true;
            mode = 3;
            break;

        default:
            printDisplayln("Err", 1);
            mode = maxMode;
            break;
    }
}

void menuSetTemp()
{
    if (isCheckGreater % 2)
    {
        greater = '>';
    }
    else
    {
        greater = '<';
    }  
    sprintf(displayBuffer, "Set:%c%-4d%c%c", greater, setTemp, ch_Degree, symB);
    printDisplayBuffer(0);
    controlValue(&setTemp, &isCheckGreater, &isTempSet, 1, 5);
}

void menuGetTemp()
{
    if (controlCancel(5))
    {
        return;
    }
    printDisplayln("Temperature", 0);
    printDisplayTempByIndex(0);
}

void menuCheckGetTemp()
{
    if (!isAlarmSet)
    {
        if (controlCancel(5))
        {
            return;
        }
    }
    
    if (isCheckGreater % 2)
    {
        if (nowTemp >= setTemp || isAlarmSet)
        {
            isAlarmSet = true;
            menuTempAlarm(0);
            return;
        }
    }
    else
    {
        if (nowTemp <= setTemp || isAlarmSet)
        {
            isAlarmSet = true;
            menuTempAlarm(0);
            return;
        }
    }

    sprintf(displayBuffer, "Set:%c%-4d%c%c", greater, setTemp, ch_Degree, symB);
    printDisplayBuffer(0);
    printDisplayTempByIndex(0);
}

void menuTempAlarm(int index)
{
    if (controlCancel(3))
    {
        return;
    }
    digitalWrite(LED_LIGHT, HIGH);
    printDisplayln("Set Alarm Out...", 0);
    printDisplayTempByIndex(index);
    if (!isPlayTone)
    {
        playTone();
    }
}
//UI


//Alarm play (WIP)
// Octave 0 Note Codes
#define       NOTE_C_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000000)
#define      NOTE_CS_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000001)
#define       NOTE_D_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000010)
#define      NOTE_DS_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000011)
#define       NOTE_E_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000100)
#define       NOTE_F_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000101)
#define      NOTE_FS_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000110)
#define       NOTE_G_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00000111)
#define      NOTE_GS_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00001000)
#define       NOTE_A_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00001001)
#define      NOTE_AS_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00001010)
#define       NOTE_B_0(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00001011)

// Octave 1 Note Codes
#define       NOTE_C_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010000)
#define      NOTE_CS_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010001)
#define       NOTE_D_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010010)
#define      NOTE_DS_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010011)
#define       NOTE_E_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010100)
#define       NOTE_F_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010101)
#define      NOTE_FS_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010110)
#define       NOTE_G_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00010111)
#define      NOTE_GS_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00011000)
#define       NOTE_A_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00011001)
#define      NOTE_AS_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00011010)
#define       NOTE_B_1(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00011011)

// Octave 2 Note Codes
#define       NOTE_C_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100000)
#define      NOTE_CS_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100001)
#define       NOTE_D_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100010)
#define      NOTE_DS_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100011)
#define       NOTE_E_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100100)
#define       NOTE_F_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100101)
#define      NOTE_FS_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100110)
#define       NOTE_G_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00100111)
#define      NOTE_GS_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00101000)
#define       NOTE_A_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00101001)
#define      NOTE_AS_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00101010)
#define       NOTE_B_2(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00101011)

// Octave 3 Note Codes
#define       NOTE_C_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110000)
#define      NOTE_CS_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110001)
#define       NOTE_D_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110010)
#define      NOTE_DS_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110011)
#define       NOTE_E_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110100)
#define       NOTE_F_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110101)
#define      NOTE_FS_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110110)
#define       NOTE_G_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00110111)
#define      NOTE_GS_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00111000)
#define       NOTE_A_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00111001)
#define      NOTE_AS_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00111010)
#define       NOTE_B_3(DURATION) ( (((uint16_t)DURATION)<<8) | 0b00111011)

// Octave 4 Note Codes
#define       NOTE_C_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000000)
#define      NOTE_CS_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000001)
#define       NOTE_D_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000010)
#define      NOTE_DS_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000011)
#define       NOTE_E_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000100)
#define       NOTE_F_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000101)
#define      NOTE_FS_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000110)
#define       NOTE_G_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01000111)
#define      NOTE_GS_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01001000)
#define       NOTE_A_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01001001)
#define      NOTE_AS_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01001010)
#define       NOTE_B_4(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01001011)

// Octave 5 Note Codes
#define       NOTE_C_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010000)
#define      NOTE_CS_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010001)
#define       NOTE_D_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010010)
#define      NOTE_DS_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010011)
#define       NOTE_E_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010100)
#define       NOTE_F_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010101)
#define      NOTE_FS_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010110)
#define       NOTE_G_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01010111)
#define      NOTE_GS_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01011000)
#define       NOTE_A_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01011001)
#define      NOTE_AS_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01011010)
#define       NOTE_B_5(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01011011)

// Octave 6 Note Codes
#define       NOTE_C_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100000)
#define      NOTE_CS_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100001)
#define       NOTE_D_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100010)
#define      NOTE_DS_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100011)
#define       NOTE_E_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100100)
#define       NOTE_F_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100101)
#define      NOTE_FS_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100110)
#define       NOTE_G_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01100111)
#define      NOTE_GS_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01101000)
#define       NOTE_A_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01101001)
#define      NOTE_AS_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01101010)
#define       NOTE_B_6(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01101011)

// Octave 7 Note Codes
#define       NOTE_C_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110000)
#define      NOTE_CS_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110001)
#define       NOTE_D_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110010)
#define      NOTE_DS_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110011)
#define       NOTE_E_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110100)
#define       NOTE_F_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110101)
#define      NOTE_FS_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110110)
#define       NOTE_G_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01110111)
#define      NOTE_GS_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01111000)
#define       NOTE_A_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01111001)
#define      NOTE_AS_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01111010)
#define       NOTE_B_7(DURATION) ( (((uint16_t)DURATION)<<8) | 0b01111011)

// Octave 8 Note Codes
#define       NOTE_C_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000000)
#define      NOTE_CS_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000001)
#define       NOTE_D_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000010)
#define      NOTE_DS_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000011)
#define       NOTE_E_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000100)
#define       NOTE_F_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000101)
#define      NOTE_FS_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000110)
#define       NOTE_G_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10000111)
#define      NOTE_GS_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10001000)
#define       NOTE_A_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10001001)
#define      NOTE_AS_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10001010)
#define       NOTE_B_8(DURATION) ( (((uint16_t)DURATION)<<8) | 0b10001011)

#define    NOTE_SILENT(DURATION) ((((uint16_t)DURATION)<<8) | 0b00001111)

static const uint16_t MelodyData[] PROGMEM = {
    NOTE_A_0(255),  NOTE_SILENT(255),   NOTE_A_0(255),  NOTE_SILENT(255),
    NOTE_A_0(255),  NOTE_SILENT(255),   NOTE_A_0(255),  NOTE_SILENT(255),
    NOTE_A_0(255),  NOTE_SILENT(255),   NOTE_A_0(255),  NOTE_SILENT(255),
    NOTE_A_0(255),  NOTE_SILENT(255),   NOTE_A_0(255),  NOTE_SILENT(255),
    NOTE_A_0(255),  NOTE_SILENT(255),   NOTE_A_0(255),  NOTE_SILENT(255),
    
};

static const uint16_t MelodyLength = sizeof(MelodyData) / sizeof(uint16_t);

static uint8_t tempo = 1;
void playTone()
{
    isPlayTone = true;
    // 8th Octave Frequencies C8 to B8, lower octaves are calculated from this
    static const uint16_t Freq8[] PROGMEM = { 4186 , 4435 , 4699  , 4978 , 5274 , 5588 , 5920 , 6272 , 6645 , 7040 , 7459 , 7902 };

    for(uint16_t  x = 0; x < MelodyLength; x++)
    {
        if (controlCancel(3))
        {
            noTone(tonePin);
            return;
        }
        uint16_t data = pgm_read_word((uint16_t *)&MelodyData[x]);
        if((data & 0xF) == 0xF) 
        {
            noTone(tonePin);
        }
        else
        {
            uint16_t Freq = pgm_read_word(&Freq8[data&0xF]) / ( 1 << (8-(data>>4 & 0xF)) );
            tone(tonePin, Freq);    
        }
        if (x == MelodyLength - 1)
        {
            x = 0;
        }   
    
    int16_t Duration = data>>8;
    while(Duration--) delay(tempo);
    }
    isPlayTone = false;
}
//Alarm play (WIP)


//misc
byte getButton()
{
    short key = analogRead(CON_BUTTON);
    #if (debugSerialPrint)
    Serial.println(key);
    #endif

    if (key > CON_VAR_NONE) return CON_BTN_NONE;

    if (key < CON_VAR_RIGHT)   return CON_BTN_RIGHT;
    if (key < CON_VAR_UP)  return CON_BTN_UP;
    if (key < CON_VAR_DOWN)  return CON_BTN_DOWN;
    if (key < CON_VAR_LEFT)  return CON_BTN_LEFT; 
    if (key < CON_VAR_SELECR)  return CON_BTN_SELECT;

    return -1;
}

float getTempByIndex(byte index)
{
    tempSensor.requestTemperatures();
    if (!isUseF)
    {
        return tempSensor.getTempCByIndex(index);
    }
    else
    {
        return tempSensor.getTempFByIndex(index);
    }
}
//misc
