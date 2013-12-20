/*
 *  Cooling process
 */

//-----------------------------------------------------------------------------/
// Cooling print functions
//-----------------------------------------------------------------------------/

// Print base cooling step
void printCooling() {
  lcdPrint(0, 0, "Cooling step        ");
  lcdPrint(1, 0, "Time:               ");
  if (VESSEL_COOLING) {
    lcdPrint(2, 0, "Temp:    C          ");
    lcdPrint(3, 0, " N.a. | N.a. | N.a. ");
  } else lcdPrint(3, 0, " Next | N.a. | N.a. ");
}

void printCoolingTime(unsigned long myTime) {
  lcdCursor(1, 6);
  lcdTime(millToMin(myTime),millRemainingSec(myTime));
}

void printCoolingTemp(int myTemp) {
  lcdCursor(2, 6);
  lcdTens(myTemp);
}

//-----------------------------------------------------------------------------/
// Cooling functions
//-----------------------------------------------------------------------------/

void startCooling() {
  alarmPrint("Start cooling wort");
}

void executeCooling() {
  unsigned long initMillis = millis();
  unsigned long currentMillis;
  int readInput;
  float myTemp;

  printCooling();
  if (VESSEL_COOLING){
    // Required for frontend
    output = 0;
    setpoint = COOLED_WORT;
  }
  
  while (true) {
    if (VESSEL_COOLING){
      myTemp = thermometerRead();
      printCoolingTemp(myTemp);
      // Check if the cooling is done
      if (myTemp < COOLED_WORT) break;
      // Required for frontend
      input = myTemp;
      myPIDUpdateFrontend();
    }

    currentMillis = millis();
    printCoolingTime(currentMillis-initMillis);

    if (!VESSEL_COOLING){
      readInput = isPressedOneSecond(1);
      if (readInput > 0) break; // Exits the while loop and ends the function
    }
  }
  alarmPrintTime("Wort cooled", millToMin(currentMillis-initMillis));
}
