/*
 *  Mash process
 */

//-----------------------------------------------------------------------------/
// Time conversion functions
//-----------------------------------------------------------------------------/

int millToMin(unsigned long millisec) {
  return int(millisec/1000/60);
}

int millRemainingSec(unsigned long millisec) {
  return (millisec/1000) % 60;
}

unsigned long minToMill(int minutes) {
  // We change the type of minutes from int to unsigned long first
  // If this is not done, the operation result*60*1000 goes over the limit
  // of int (65536)
  unsigned long result = minutes;
  result = result*60*1000;
  return result;
}

//-----------------------------------------------------------------------------/
// Generic print functions
//-----------------------------------------------------------------------------/

void printStep() {
  lcdPrint(0, 0, "     step           ");
  lcdPrint(1, 0, "Time:               ");
  lcdPrint(2, 0, "Temp:     C         ");
  lcdPrint(3, 0, "Power: |          | ");
}

void printStepNb(int step) {
  lcdCursor(0, 10);
  lcdTens(step);
}

void printStepTime(int actTime) {
  lcdCursor(1, 6);
  lcdTens(actTime);
}

void printTotalTime(int objTime) {
  lcdCursor(1, 13);
  lcdTens(objTime);
}

void printStepTimeD(unsigned long objMillis) {
  lcdCursor(1, 6);
  lcdTime(millToMin(objMillis),millRemainingSec(objMillis));
}

void printTotalTimeD(unsigned long actMillis) {
  lcdCursor(1, 13);
  lcdTime(millToMin(actMillis),millRemainingSec(actMillis));
}

void printActualTemp(float actTemp) {
  lcdCursor(2, 6);
  lcdFloat(actTemp);
}

//-----------------------------------------------------------------------------/
// Mash print functions
//-----------------------------------------------------------------------------/

void printMash() {
  lcdPrint(0, 0, "Mash");
  lcdPrint(2, 12, "->     C");
}

void printObjectiveTemp(float objTemp) {
  lcdCursor(2, 15);
  lcdFloat(objTemp);
}

void printMashHeating() {
  lcdPrint(0, 0, "Preparing mash step ");
  lcdPrint(1, 0, "Heating water       ");
}

void printMashPower() {
  int power = output*20/windowSize; // Returns a number btw 1-20

  lcdCursor(3, 8);
  for (int i=0;i<power/2;i++) lcd.print("O");
  if (power%2 == 0 && power < 20) lcd.print("_");
  if (power%2 == 1) lcd.print("C");
  for (int i=power/2;i<20/2-1;i++) lcd.print("_");
}

//-----------------------------------------------------------------------------/
// Mash functions
//-----------------------------------------------------------------------------/

void checkWater() {
  alarmPrint("Check pot has water");
}

// Increase water temperature above target temperature before inserting grains
void raiseMashTemp() {
  float myTemp;
  int objTemp = firstMashStep->temperature + WATER_ADDITIONAL_TEMP;

  myPIDStartStep(objTemp);

  // This is step 0, as we are simply heating the water
  // before inserting the grain
  printStep();
  printMash();
  printMashHeating();
  printObjectiveTemp(objTemp);
  printMashPower();
  
  while (true) {
    myTemp = thermometerRead();
    printActualTemp(myTemp);
    myPIDLoop();
    myPIDUpdateFrontend();
    printMashPower();
    // Check if we have arrived at target temperature
    if (myTemp >= objTemp) break;
  }
  
  myPIDStopStep();
}

void addGrain() {
  alarmPrint("Add grain");
}

void executeMashStep(int stepID, int temperature, int minutes, int minutesTot) {
  unsigned long milliseconds = minToMill(minutes);
  unsigned long millisTot = minToMill(minutesTot);
  unsigned long initMillis = millis();
  unsigned long currentMillis, leftMillis;
  float myTemp;
  
  myPIDStartStep(temperature);

  printStep();
  printMash();
  printStepNb(stepID);
  printObjectiveTemp(temperature);
  printStepTimeD(milliseconds);
  printTotalTimeD(millisTot);
  printMashPower();

  while (true) {
    myTemp = thermometerRead();

    currentMillis = millis();
    leftMillis = initMillis + milliseconds - currentMillis;

    printStepTimeD(leftMillis);
    printTotalTimeD(millisTot-milliseconds+leftMillis);
    printActualTemp(myTemp);
    
    // leftMillis is an unsigned long. As such, it does not have negative
    // values, instead it loops and returns a very large positive
    // integer (4,294,967,295 or 2^32-1). To offset this effect, we stop the
    // step as soon as less than a certain number of seconds are left.
    if (leftMillis <= 1000) break; // Exits the while(true) loop
    myPIDLoop();
    myPIDUpdateFrontend();
    printMashPower();
    delay(250); // No need to check the PID loop too often
  }
  myPIDStopStep();
}

void executeMash() {
  mashStep *thisMashStep = firstMashStep;

  while (thisMashStep != NULL) {
    executeMashStep(thisMashStep->stepID,
                    thisMashStep->temperature,
                    thisMashStep->minutes,
                    mashTimeStep(thisMashStep));
    thisMashStep = thisMashStep->nextStep;
  }
}

void checkGrainRemoved() {
  alarmPrint("Remove grain");
}
