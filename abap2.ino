/*
 *	ABAP - ACCE brewery automation project
 *
 * 	Automates the brewing process by allowing to control the temperature
 *      and duration of mashing steps.
 * 
 * 	v0.1: 1 june 2012
 * 	by Humphrey <NBED email?>
 *
 * 	v0.2: september 2012
 * 	by Guillermo Fernandez Castellanos <guillermo@guindilla.com>
 *      - rewrote code based on v0.1 program
 *
 *      v0.9: june 2013
 *      by Guillermo Fernandez Castellanos <guillermo@guindilla.com>
 *      - rewrote code based on v0.2 program
 *      - added rotary switch as input
 *      - improved input as buttons
 *      - moved LCD to I2C bus
 *      - added cooling step timer and control
 *      - for 2 resistances, they get activated in alternation
 *      - renamed all files to avoid dependencies problems when compiled
 *
 *      This project is based on external libraries that must be
 *      downloaded separately:
 *      - Bounce.h:
 *        http://playground.arduino.cc/code/bounce
 *      - DallasTemperature.h:
 *        https://github.com/milesburton/Arduino-Temperature-Control-Library
 *      - Encoder.h:
 *        http://www.pjrc.com/teensy/td_libs_Encoder.html
 *      - NewLiquidCrystal (verify link):
 *        https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
 *      - OneWire.h:
 *        http://playground.arduino.cc/Learning/OneWire
 *      - PID_v1.h:
 *        https://github.com/br3ttb/Arduino-PID-Library
 *      - PID_AutoTune_v0.h:
 *        https://github.com/br3ttb/Arduino-PID-AutoTune-Library
 *
 *      The naming of the files of this project are due to the specificities
 *      of Arduino's build process. More information on those problems can be
 *      found here:
 *     http://arduino.cc/en/Hacking/BuildProcess
 *     http://liudr.wordpress.com/2011/02/16/using-tabs-in-arduino-ide/
 *     http://arduinodilettante.wordpress.com/2011/03/10/multiple-file-sketches/
 *
 */

// placed here to avoid problems when compiling
#include <Bounce.h>

//-----------------------------------------------------------------------------/
// Arduino pins
//-----------------------------------------------------------------------------/

// Define button pins: BUTTON1, BUTTON2
const int buttonPins[] = {8,9};

// Change these two numbers to the pins connected to your encoder.
//  - Best Performance: both pins have interrupt capability
//                      (2 and 3 for Arduino UNO)
//  - Good Performance: only the first pin has interrupt capability
//  - Low Performance:  neither pin has interrupt capability
// avoid using pins with LEDs attached
const int rotaryPins[] = {2,3};

// Pin where water resistance is connected
// Set resistancePin2 to 0 if no 2nd resistance is set
const int resistancePin1 = 11;
const int resistancePin2 = 12;

// Pin where temperature probe is connected
const int thermometerPin = 9;

// Pin where alarm is connected (LED, buzzer,...)
const int alarmPin = 13;

// IMPORTANT: The LCD MUST be connected as such: SDA->A4 SCL->A5

//-----------------------------------------------------------------------------/
// LCD variables
//-----------------------------------------------------------------------------/

const byte I2C_ADDR = 0x20;  // Define I2C Address of the LCD

//-----------------------------------------------------------------------------/
// Button variables
//-----------------------------------------------------------------------------/

const int DEBOUNCE_TIME = 5;      // Debounce time in ms

//-----------------------------------------------------------------------------/
// Mash, boil and cooling configuration defaults
//-----------------------------------------------------------------------------/

// Default times assigned to mash and boil steps when created
const int TIME_DEFAULT = 30;
const int TEMP_DEFAULT = 65;

// ºC above target temperature to compensate for loss of tmp when inserting malt
const int WATER_ADDITIONAL_TEMP = 5;

// ºC at which the PID considers the water is boiling
// Is unlikely to be 100 due to calibration of temp probes and/or brewer being
// at high altitude
const int BOIL_TEMP = 98;

// Power at which the resistances will be powered once the water arrives to the
// boil. Choose a number between 0 and 20.
const int BOIL_POWER = 10;

// Temperature in ºC at which the wort is considered cooled
const int COOLED_WORT = 22;

// Type of cooling:
// - true: the whole wort is cooled simultaneously, such as with a wort chiller
// - false: only part of the wort is cooled at a time, such as with a
//   counterflow chiller or a plater chiller
const boolean VESSEL_COOLING = true;

//-----------------------------------------------------------------------------/
// PID settings
//-----------------------------------------------------------------------------/

// ºC below target temperature before activating PID
// Before that, all resistances will be activated at 100%
const int PID_ACTIVATE_TEMP = 5;

// Window of time that the PID considers to activate or deactivate resistances
int windowSize = 5000; // miliseconds

//const double Kp = 1250; // (P)roportional tuning parameter
// const double Ki = 2;   // (I)ntegral tuning parameter
// const double Kd = 0;   // (D)erivative tuning parameter
const double Kp = 3900; // (P)roportional tuning parameter
const double Ki = 20;   // (I)ntegral tuning parameter
const double Kd = 0;    // (D)erivative tuning parameter

// minimum number of milliseconds that the PID must indicate for the program
// to start the water resistance. This is done to avoid on-off cycles of a few
// milliseconds that might damage the resistance.  
int MIN_ON_RESISTANCE = 500;

//-----------------------------------------------------------------------------/
// Main program
//-----------------------------------------------------------------------------/

void setup(void)
{
  // start serial port for either frontend or debug
  Serial.begin(9600);
  buttonsSetup();
  lcdSetup();
  alarmSetup();
  thermometerSetup();
  myPIDSetup();
  resistancesSetup();
}

void loop(void)
{
  int choice;
  choice = lcdInitBrewing(2000);

  if (choice == 0) {
    // Default choice
    configureMash();
    mashPrintSteps();
    configureBoil();
    checkWater();
    raiseMashTemp();
    addGrain();
    executeMash();
    checkGrainRemoved();
    raiseBoilTemp();
    executeBoil();
    startCooling();
    executeCooling();
    lcdEndBrewing();    
  } else if (choice == 1) {
    checkWater();
    executeManualMash();
  } else if (choice == 2) {
    checkWater();
    executeManualBoil();
  }
}
