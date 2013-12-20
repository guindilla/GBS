/*
 *  Management of inputs
 */

//-----------------------------------------------------------------------------/
// Buttons
//-----------------------------------------------------------------------------/

// Without hardware debouncer
// +5V ---button---RRRR---GND
//               |
//               |
//              PIN
// RRRR = 10kO
//
// With hardware debouncer
//      -button-
//      |      |
//      |      |
// +5V ---CCCC---RRRR---GND
//             |
//             
//            PIN
//
// CCCC = 100nF
// RRRR = 10kO

// This include was included in abap2.ino to avoid problems when compiling
//#include <Bounce.h>

// How many buttons do we have?
int nbButtons = sizeof(buttonPins)/sizeof(int);
Bounce bouncer1 = Bounce(buttonPins[0],DEBOUNCE_TIME);
Bounce bouncer2 = Bounce(buttonPins[1],DEBOUNCE_TIME);

// Setup function
void buttonsSetup() {
  for (int i=0; i<nbButtons; i++){
      pinMode(buttonPins[i],INPUT);
  }
}

// Returns a pointer to the adequate Bounce object or NULL if error
// NBED Ideally, this should be dynamic based on an array of Bounce objects
Bounce *selectButton(int buttonID) {
  // Check validity of inputs
  if (buttonID>nbButtons || buttonID<1) return NULL;
  switch (buttonID) {
    case 1:
      return &bouncer1;
    case 2:
      return &bouncer2;
    default:
      return NULL;
  }
}

// Returns 0 if button not pressed, 1 if pressedfor less than 1 second,
// 2 if pressed for more than 1 second, and -1 if error
int isPressedOneSecond(int buttonID){
  Bounce *tmpBounce = NULL;
  tmpBounce = selectButton(buttonID);
  if (tmpBounce == NULL) return -1;
  // Has the button been pushed lately?
  if (tmpBounce->update() && tmpBounce->risingEdge()) {
    unsigned long thisTime = millis();
    // If so, keep checking until we know for how long.
    while (!tmpBounce->update()) {};
    thisTime = millis() - thisTime;
    if (thisTime < 1000) {return 1;} else {return 2;}
  } else {
    return 0;
  }
}

//-----------------------------------------------------------------------------/
// Rotary switch
//-----------------------------------------------------------------------------/

// Load Libraries
#include <Encoder.h>

Encoder myEncoder(rotaryPins[0], rotaryPins[1]);

// Returns the difference between the encoder's last read and the present one
int readEncoder() {
  int newPosition = myEncoder.read();
  myEncoder.write(0);
  delay(50);
  return newPosition/2;
}
