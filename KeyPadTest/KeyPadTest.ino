/* @file CustomKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates changing the keypad size and key values.
|| #
*/
#include <Keypad.h>

// Setup Keypad variables
#define ROWS 5 //Zeilen
#define COLS 4 //Spalten

// green KeyPad 5x4
char keys[ROWS][COLS] = {
  {'P','R','0','L'}, //TrackPower,Right,0,Left
  {'E','9','8','7'}, //ESC,9,8,7
  {'D','6','5','4'}, //Down,6,5,4
  {'U','3','2','1'}, //Up,3,2,1
  {'*','#','B','A'}  //*,#,F2,F1
};

byte colPins[COLS] = {5, 4, 3, 2};          //connect to the row pinouts of the keypad
byte rowPins[ROWS] = {6, 7, 8, 9, 10};      //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    Serial.println(customKey);
  }
}
