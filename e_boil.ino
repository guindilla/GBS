/*
 *  Boil process
 */

//-----------------------------------------------------------------------------/
// Boil print functions
//-----------------------------------------------------------------------------/

void printBoil() {
  lcdPrint(0, 0, "Boil");
}

// Power argument should be between 0 and 20
void printBoilPower(int power) {
  // Arguments within range?
  int myPower = constrain(power, 0, 20);

  lcdCursor(3, 8);
  for (int i=0;i<power/2;i++) lcd.print("O");
  if (myPower%2 == 0 && myPower < 20) lcd.print("_");
  if (myPower%2 == 1) lcd.print("C");
  for (int i=myPower/2;i<20/2-1;i++) lcd.print("_");
}

void printBoilHeating() {
  lcdPrint(0, 0, "Preparing boil step ");
  lcdPrint(1, 0, "Heating water       ");
}

//-----------------------------------------------------------------------------/
// Boil functions
//-----------------------------------------------------------------------------/

// Increase water temperature to boil as defined by BOIL_TEMP variable
void raiseBoilTemp() {
  float myTemp;

  myPIDStartStep(BOIL_TEMP);

  // This is step 0, as we are simply heating the water until it boils
  printStep();
  printBoil();
  printBoilHeating();
  resistancesOn();
  printBoilPower(resistancesPower);

  while (true) {
    myTemp = thermometerRead();
    printActualTemp(myTemp);
    myPIDUpdateFrontend();
    // Check if we have arrived at target temperature
    if (myTemp >= (float) BOIL_TEMP) break;
  }
  myPIDStopStep();
}

void executeBoilStep(int stepID, int minutes, int minutesTot) {
  unsigned long milliseconds = minToMill(minutes);
  unsigned long millisTot = minToMill(minutesTot);
  unsigned long initMillis = millis();
  unsigned long currentMillis, leftMillis;
  float myTemp;

  // This is step 0, as we are simply heating the water
  // to arrive to boiling temperature
  printStep();
  printBoil();
  printStepNb(stepID);
  printStepTimeD(milliseconds);
  printTotalTimeD(millisTot);
  printBoilPower(resistancesPower);

  #ifdef M_DEBUG
    Serial.print("executeBoilStep() - initMillis ");
    Serial.println(initMillis);
    Serial.print("executeBoilStep() - thisBoilStep->minutes ");
    Serial.println(minutes);
  #endif
    
  while (true) {
    myTemp = thermometerRead();

    currentMillis = millis();
    leftMillis = initMillis + milliseconds - currentMillis;

    printStepTimeD(leftMillis);
    printTotalTimeD(millisTot-milliseconds+leftMillis);
    printActualTemp(myTemp);

    #ifdef M_DEBUG
      Serial.print("executeBoilStep() - leftMillis ");
      Serial.println(leftMillis);
    #endif

    // leftMillis is an unsigned long. As such, it does not have negative
    // values, instead it loops and returns a very large positive
    // integer (4,294,967,295 or 2^32-1). To offset this effect, we stop the
    // step as soon as less than a certain number of seconds are left.
    if (leftMillis <= 1000) break; // Exits the while(true) loop
    
    // Check if there has been any change by the user in the power it wants
    // to flow through the resistances.
    resistancesPower += readEncoder();
    resistancesPower = constrain(resistancesPower,0,20);

    resistancesLoop();
    myPIDUpdateFrontend();
    printBoilPower(resistancesPower);
    
    delay(250); // No need to check the PID loop too often
  }
}

void addAditions() {
  alarmPrint("Add additions");
}

void executeBoil() {
  boilStep *thisBoilStep = firstBoilStep;

  resistancesStartStep();
 
  while (thisBoilStep != NULL) {
    #ifdef M_DEBUG
      Serial.print("executeBoil() - thisBoilStep->stepID ");
      Serial.println(thisBoilStep->stepID);
      Serial.print("executeBoil() - thisBoilStep->minutes ");
      Serial.println(thisBoilStep->minutes);
    #endif
    executeBoilStep(thisBoilStep->stepID,
                    thisBoilStep->minutes,
                    boilTimeStep(thisBoilStep));
    thisBoilStep = thisBoilStep->nextStep;
    if (thisBoilStep != NULL) addAditions();
  }
  // End of boil, turn resistances off
  resistancesOff();
}

void endOfBoil() {
  alarmPrint("End of boil");
}
