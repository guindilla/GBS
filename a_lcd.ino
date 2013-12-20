/*
 *  LCD funcions
 */

// Load Libraries
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// Format of LCD in lines and columns
const int LCD_COLUMNS = 20;
const int LCD_ROWS = 4;

//Initialise the LCD
const int BACKLIGHT_PIN = 3;
const int En_pin = 2;
const int Rw_pin = 1;
const int Rs_pin = 0;
const int D4_pin = 4;
const int D5_pin = 5;
const int D6_pin = 6;
const int D7_pin = 7;
LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

void lcdSetup() {
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
}

// Check that this works with all configurations
void lcdPrint(int line, int column, const char* message) {
  lcd.setCursor(column,line);
  lcd.print(message);
}

// Set cursor to the specified position
void lcdCursor(int line, int column) {
  lcd.setCursor(column,line);
}

// Clear the screen
void lcdClear() {
  lcd.clear();
}

//-----------------------------------------------------------------------------/
// Support functions
//-----------------------------------------------------------------------------/

void lcdTens(int number) {
  if (number < 10) lcd.print("0");
  lcd.print(number);
}

void lcdHundreds(int number) {
  if (number < 100) lcd.print(" ");
  if (number < 10) lcd.print("0");
  lcd.print(number);
}

void lcdTime(int minutes, int seconds) {
  lcdHundreds(minutes);
  lcd.print(":");
  lcdTens(seconds);
}

void lcdFloat(float number) {
  int intNum = number;
  int floatNum = number * 10 - intNum * 10;
  
  lcdTens(intNum);
  lcd.print(".");
  lcd.print(floatNum);
}

//-----------------------------------------------------------------------------/
// Splash screens
//-----------------------------------------------------------------------------/

// Welcoming message, checks if there are any special programs being launched
// Returns the number of the button being pressed for more than 1 second,
// 0 otherwise
int lcdInitBrewing(int delayMs) {
  unsigned long initTime = millis();
  unsigned long now;
  int readInput;

  lcdPrint(0, 0, "     -- ABAP --     ");
  lcdPrint(1, 0, "    ACCE Brewery    ");
  lcdPrint(2, 0, " Automation Project ");
  lcdPrint(3, 0, "      Welcome       ");

  while (true) {
    now = millis();
    if (now > delayMs) break;

    for (int i = 0; i < nbButtons; i++) {
      readInput = isPressedOneSecond(i+1);
      if (readInput == 2) return i+1;
    }
  }
  return 0;
}

// Shutdown message
void lcdEndBrewing() {
  lcdPrint(0, 0, "     -- ABAP --     ");
  lcdPrint(1, 0, "    ACCE Brewery    ");
  lcdPrint(2, 0, " Automation Project ");
  lcdPrint(3, 0, "  End brew session  ");
  delay(3600000); // 1h
}
