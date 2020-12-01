                                                                                                              /*
  Mark Fox's DCC++ 'Bodnar Throttle'.  I think I might brand it a 'Bodnar'.  :)
  Rewritten and customised from Dave Bodnar's code from June 16th, 2016, his version 2.6a
  Version 1.00 uses an Arduino MEGA2560 (devboard) with a 4x4 keypad, 20x4 I2C LCD Display, and uses digital debouncing on the KY-040 rotary encoder without an interrupt.
  This version is 1.03, a heavy edit streamlining all the function comms.
  Date 6th August 2020.

  Changes by Franziska Walter
  2020-11-27
  v2.2.5FW
*/
#define VERSION "2.2.5FW"
#define VDATE "2020-11-27"

#define DEVICE_ID 1     // Callback Id  -> each Device need other ID
#define I2C_ADDR   0x27 // or 0x3F

#include "Arduino.h"
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Pin definitions
// Rotary encoder Pins
#if (defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560))
  //Mega
  // overwrite defines ...
  #define DEVICE_ID 2     // Callback Id 
  #define I2C_ADDR   0x3F
  
  #include <SPI.h>
  #include <SD.h>
  
  #define SD_CHIP_SELECT 53 // Pin ChipSelect - SS
  #define mySerial Serial1

  File myFile;
  int DecoderValue;
  char Manufacturer[19];
  char DecoderTyp[19];
  long CVType;
  int CVVersion;
  int CVSubversion;
  int CVBuild;
  int ValVersion;
  int ValSubversion;
  int ValBuild;
  char ExtFile[19];
#else
  //Nano or Uno
  // overwrite defines ...
  #define DEVICE_ID 1     // Callback Id 
  #define I2C_ADDR   0x27 //0x3F
  
  #define mySerial Serial
#endif
bool isSDCARD = false;

#define maxLocos 4
#define reverse_direction 5 // counts rotary encoder at zero to reverse direction
#define reverse_timeout 1000

#define RE_CLK    A0
#define RE_DATA   A1
#define RE_Button A2
#define reverse_rotary 0    // if up/down in wrong direction

#define rotary_push_from_zero 75  //set speed value push on zero -> = 0 nothing to do
#define dont_change_smaller 45    //dont save changes if speed value smaller than value
unsigned long lastRotPush = millis();
unsigned long PushRotTimeout = 100;
unsigned long lastKeyPush = millis();
unsigned long PushKeyTimeout = 100;

// Display I2C Address, use i2c_scanner, find out Address
// Pins on Arduino Nano: A4 = SDA, A5 = SCL
// Arduino Mega: D20 = SDA, D21 = SCL
// #define I2C_ADDR   0x27 //0x3F


#define debug 0           // set to 1 to show debug info on serial port - assume that it will cause issues with DCC++ depending on what is sent

// https://maxpromer.github.io/LCD-Character-Creator/
uint8_t zerodot[8] = {0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0};
uint8_t onedot[8] = {0x0, 0xe, 0x1f, 0x1b, 0x1f, 0xe, 0x0};
uint8_t pushdot[8] = {0x0, 0x0, 0xA, 0x4, 0xA, 0x0, 0x0};
uint8_t trackoff[8] = {0x0e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0e, 0x00};

// Setup Keypad variables
#define ROWS 5 //Zeilen
#define COLS 4 //Spalten

// green KeyPad 5x4
char keys[ROWS][COLS] = {
  {'P','R','0','L'}, //TrackPower, Right,  0, Left
  {'E','9','8','7'}, //       ESC,     9,  8,    7
  {'D','6','5','4'}, //      Down,     6,  5,    4
  {'U','3','2','1'}, //        Up,     3,  2,    1
  {'*','#','B','A'}  //         *,     #, F2,   F1
};

byte colPins[COLS] = {5, 4, 3, 2};          //connect to the row pinouts of the keypad
byte rowPins[ROWS] = {6, 7, 8, 9, 10};      //connect to the column pinouts of the keypad

bool track_power = false;
int LocoAddress[maxLocos] = {1111, 2222, 3333, 4444};
int LocoDirection[maxLocos] = {1, 1, 1, 1};
int LocoSpeed[maxLocos] = {0, 0, 0, 0};
int LocoZeroCount[maxLocos] = {reverse_direction, reverse_direction, reverse_direction, reverse_direction};
unsigned long LocoZeroTimeout[maxLocos] = {0, 0, 0, 0};
bool LocoInUse[maxLocos] = {false, false, false, false};
unsigned long LocoFunction[maxLocos] = {0, 0, 0, 0};
unsigned long LocoPushFunction[maxLocos] = {0, 0, 0, 0};
int LocoMaxSpeed[maxLocos] = {rotary_push_from_zero, rotary_push_from_zero, rotary_push_from_zero, rotary_push_from_zero};
// ontime, addr, function
unsigned long LocoPushOff[5][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};
int next_push_idx = 0;
unsigned long PushTimeout = 1000;

int ActiveAddress = 0; // make address1 active
int CVLine = 1; //  1 - Address, 2 - Value, 3 - Value Bin
int CVNumber = 0;
char CVNumberArr[5] = {'0','0','0','0','\0'};
int CVVal = 0;
char CVValArr[4] = {'0','0','0','\0'};
int CVCol = 4;    // wenn spalte direkt angewählt wird, dann nur einzeln hoch/runterzählen
char CVBin[8] = {0,0,0,0,0,0,0,0};
int CVOne = 0;  // one count up/down
unsigned long lastCV = millis();
unsigned long CVTimeout = 5000;
int CVSend = 0;
int UpdateCVModus = 0;
int lastUpdateCVModus = 0;
int DecoderAddr = 1;    //Decoder Read/write Address
char DecoderAddrArr[5] = {'0','0','0','1','\0'};
bool DecoderLong = false; //Long/Short
bool DecoderSpeedStep = true; // 14 <-> 28/126

// Prog Callbacks here defined
const int CB_NUM_DEFAULT = 1;
const int CB_NUM_GETADDR = 2;   //GetAddress
const int CB_NUM_SETADDR = 3;   //SetAddress
const int CB_NUM_GETINFO = 4;   //Get Decoder Info
const int CB_NUM_GETLOCODATA = 5;   //Get LocoData
const int CB_READ_BYTE = 10;    // Bit 10 is set
const int CB_WRITE_BYTE = 11;   // Bit 11 is set

unsigned long lastDatas = millis();
unsigned long DataTimeout = 1000;

// Now setup the hardware
LiquidCrystal_I2C lcd(I2C_ADDR, 20, 4); // Setup LCD
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

static uint8_t prevNextCode = 0; // statics for rotary encoder
static uint16_t store = 0;

int buttonState = 0;
int re_absolute = 0;
char key ;

const int fail_count = 5; // after n fails message disconnected
int cmd_count = fail_count;     // counter for incoming data
bool cmd_connected = false;
int active_modus = 2; // 0 - normal, 1 - Prog Functions, 2 - no connection
int last_modus = 0;
bool CVPROG = false;
int CVPROGMODE = 0;   // 0 - Prog, 1 - POM, 2 - LOCO Addr, 3 - LOCO Info
int CVPROGMODE_MAX = 2;   // welche Funktionen sind max. nutzbar , 3 nur mit dem Mega + SD-Card

/* Stream Parser Definitions */
const int MAX_PARAMS=10;  // Must not exceed this
const int MAX_BUFFER=50;  // longest command sent in
byte  bufferLength=0;
bool  inCommandPayload=false;
char  buffer[MAX_BUFFER+2]; 
int splitValues( int result[MAX_PARAMS], const byte * command);

const int HASH_KEYWORD_PROG=-29718;
const int HASH_KEYWORD_MAIN=11339;
const int HASH_KEYWORD_JOIN=-30750;
/* Stream Parser Definitions */

void setup() {

  //Setup Encoder Here
  pinMode(RE_CLK, INPUT);
  pinMode(RE_CLK, INPUT_PULLUP);
  pinMode(RE_DATA, INPUT);
  pinMode(RE_DATA, INPUT_PULLUP);
  pinMode(RE_Button, INPUT);
  pinMode(RE_Button, INPUT_PULLUP);
  lcd.backlight();
  lcd.init();
  lcd.createChar(0, zerodot);
  lcd.createChar(1, onedot);
  lcd.createChar(2, trackoff);
  lcd.createChar(3, pushdot);
  lcd.home (); // go home
  mySerial.begin (115200);
  //#if debug > 0
    Serial.begin (115200);
  //#endif

  #if (defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560))  
    if (SD.begin(SD_CHIP_SELECT)) {
      isSDCARD = true;
      CVPROGMODE_MAX = 3;
    }
  #endif
  
  if (hasData(1)) {
    getAddresses(1);  // read loco IDs from eeprom
    getPushFunctions(1); //read functions from eeprom
    getLastProg(1);
  } else {
    // store default data
    saveAddresses(1);  // read loco IDs from eeprom
    savePushFunctions(1); //read functions from eeprom
    saveLastProg(1);
  }
  
  lcd.print("DCC++ Throttle");
  lcd.setCursor(0, 1);
  lcd.print(VDATE);
  lcd.print(" v");
  lcd.print(VERSION);
  lcd.setCursor(0, 2);
  //lcd.print("Push Button Function");
  lcd.print("--------------------");
  lcd.setCursor(0, 3);
  lcd.print("Btn:");
  lcd.print(" 0->");
  lcd.print(rotary_push_from_zero);
  lcd.print(" X->0 ");
  if (isSDCARD) {
    lcd.setCursor(18, 3);
    lcd.print("SD");
  }
  #if debug == 1
    Serial.print(VDATE);
    Serial.print(" Version ");
    Serial.print(VERSION);
    Serial.println("");
  #endif
  delay(5000);
  lcd.clear();
  InitialiseSpeedsLCD();
  InitialiseFunctionLCD();
  lcd.setCursor(4, ActiveAddress);
  lcd.blink();

}  // END SETUP

void loop() {
  static int8_t re_val;

  StreamParser_loop(mySerial);

  if (cmd_count > fail_count) {
    cmd_count = fail_count + 1;
    cmd_connected = false;
  } else {
    if (cmd_count < 0) cmd_count = 0;
    cmd_connected = true;
  }
  // dont change progmodus
  if (cmd_connected && (active_modus == 2)) {
    active_modus = 0;
    if (!CVPROG) UpdateModusLCD();
  }
  if (!cmd_connected && (active_modus == 0)) {
    active_modus = 2;
    if (!CVPROG) UpdateModusLCD();
  }
  if (CVPROG) UpdateCVModusLCD(false);

  if (millis() - lastKeyPush  > PushKeyTimeout) {
    // First check the keypad
    key = keypad.getKey();
  
    if (key) {
      #if debug > 0
        Serial.println(" ");
        Serial.println(key);
      #endif
  
      if (CVPROG == false) {
        loop_loco_key();
      } else {
        loop_prog_key();
      }
      
    }
    lastKeyPush = millis();
  }
  
  // Read encoder
  if ( re_val = read_rotary() ) {
    if (CVPROG == false) {
      re_absolute += re_val;
      re_absolute = constrain(re_absolute, 0, 126);
      LocoSpeed[ActiveAddress] = re_absolute;
      if (LocoSpeed[ActiveAddress] == 0) {
        if (LocoZeroCount[ActiveAddress] == 0) LocoZeroTimeout[ActiveAddress] = millis();
        LocoZeroCount[ActiveAddress]++;
        if ((LocoZeroCount[ActiveAddress] > reverse_direction) || (millis() - LocoZeroTimeout[ActiveAddress]  > reverse_timeout)) {
          // Reverse direction...
          LocoDirection[ActiveAddress] = !LocoDirection[ActiveAddress];
        }
      } else {
        LocoZeroCount[ActiveAddress] = 0;
      }
      doDCCspeed(ActiveAddress);
      updateSpeedsLCD(ActiveAddress);
    } else {
      loop_rotary_prog(re_val);
    }
  }

  if (millis() - lastRotPush  > PushRotTimeout) {
    buttonState = digitalRead(RE_Button);
    if (buttonState == LOW) {
      //delay(50);
      buttonState = digitalRead(RE_Button); // check a 2nd time to be sure
      if (buttonState == LOW) {// check a 2nd time to be sure
        if (CVPROG == false) {
          // Reverse direction...
          // LocoDirection[ActiveAddress] = !LocoDirection[ActiveAddress];
          // ... and set speed to zero (saves loco running away on slow decel/accel set in decoder.)
          if (LocoSpeed[ActiveAddress] == 0 && rotary_push_from_zero > 0) {
            LocoSpeed[ActiveAddress] = LocoMaxSpeed[ActiveAddress];
            re_absolute = rotary_push_from_zero;
          } else if (LocoSpeed[ActiveAddress] > 0) {
            if (LocoSpeed[ActiveAddress] > dont_change_smaller) {
              LocoMaxSpeed[ActiveAddress] = LocoSpeed[ActiveAddress];
            }
            LocoSpeed[ActiveAddress] = 0;
            LocoZeroCount[ActiveAddress] = 0;
            re_absolute = 0;
          }
          
          doDCCspeed(ActiveAddress);
          updateSpeedsLCD(ActiveAddress);
          lastDatas = millis() + PushTimeout;
        } else if (CVPROGMODE < 3) {
          // CV Prog
          CVCol++;
          if ((CVLine == 1) && (CVCol > 8)) CVCol = 4; //Addr
          if ((CVLine == 1) && (CVCol > 4)) CVOne = CVNumberArr[CVCol - 5] - 48; // Addr
  
          if (CVPROGMODE == 2) {
            // Decoder Addr
            CVOne = DecoderAddrArr[CVCol - 5] - 48;
          } else {
            CVOne = CVNumberArr[CVCol - 5] - 48; // Addr
          }
          
          if (CVPROGMODE < 2) {
            // normal Prog or POM
            if ((CVLine == 2) && (CVCol > 7)) CVCol = 4; //Value
            if ((CVLine == 3) && (CVCol > 12)) CVCol = 4; //Bin
            if ((CVLine == 2) && (CVCol > 4)) CVOne = CVValArr[CVCol - 5] - 48; // Value
            if ((CVLine == 3) && (CVCol > 4)) CVOne = bitRead(CVVal, 12 - CVCol); // binary
          } else if (CVPROGMODE == 2) {
            // Decoder Addr
            if (CVLine > 1 && CVCol != 4) CVCol = 4;
          }
          lcd.setCursor(CVCol, CVLine);
        }
        do {  // routine to stay here till button released & not toggle direction
          buttonState = digitalRead(RE_Button);
        } while (buttonState == LOW);
      }
    }
    lastRotPush = millis();
  }
  
  // SetPushfunction off
  for (int i = 0; i < 5; i++) {
    if ((LocoPushOff[i][0] != 0) && ((millis() - LocoPushOff[i][0])) > PushTimeout) {
        bitWrite(LocoFunction[LocoPushOff[i][1]], LocoPushOff[i][2], 0);
        doDCCfunction(LocoPushOff[i][2], LocoPushOff[i][1]);
        if (LocoPushOff[i][1] == ActiveAddress) UpdateFunctionLCD(LocoPushOff[i][2]);
        LocoPushOff[i][0] = 0;
        LocoPushOff[i][1] = 0;
        LocoPushOff[i][2] = 0;
        lastDatas = millis();
    }
  }

  if (millis() - lastDatas  > DataTimeout) {
    GetLocoDatas();
    //delay(50);
    lastDatas = millis();
  }

  // 0 - do nothing, 1 - wait for response, 2 - del message
  if (CVSend > 0) {
    if (millis() - lastCV  > CVTimeout) {
      if (CVSend == 1) {
        CVSend++;
        if (CVPROG) updateCVMsg("TimOut");
      } else {
        CVSend = 0;
        if (CVPROG) updateCVMsg(" ");
      }
      lastCV = millis();
    }
  }
  
}  //END LOOP

// Robust Rotary encoder reading
// Copyright John Main - best-microcontroller-projects.com
// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
  int tempx = ActiveAddress;
  
  prevNextCode <<= 2;
  #if reverse_rotary == 0
    if (LocoDirection[tempx] == 1 || CVPROG) {
      if (digitalRead(RE_DATA)) prevNextCode |= 0x01;
      if (digitalRead(RE_CLK)) prevNextCode |= 0x02;
    } else {
      if (digitalRead(RE_DATA)) prevNextCode |= 0x02;
      if (digitalRead(RE_CLK)) prevNextCode |= 0x01;
    }
  #else
    if (LocoDirection[tempx] == 1 || CVPROG) {
      if (digitalRead(RE_DATA)) prevNextCode |= 0x02;
      if (digitalRead(RE_CLK)) prevNextCode |= 0x01;
    } else {
      if (digitalRead(RE_DATA)) prevNextCode |= 0x01;
      if (digitalRead(RE_CLK)) prevNextCode |= 0x02;
    }
  #endif
  prevNextCode &= 0x0f;

  // If valid then store as 16 bit data.
  if  (rot_enc_table[prevNextCode] ) {
    store <<= 4;
    store |= prevNextCode;
    if ((store & 0xff) == 0x2b) return -1;
    if ((store & 0xff) == 0x17) return 1;
  }
  return 0;
}
