/*
 *  Manual processes
 */

//-----------------------------------------------------------------------------/
// Manual print functions
//-----------------------------------------------------------------------------/

void printManualMash() {
  printMash();
  lcdPrint(0, 0, "Control PID         ");
}

void printManualBoil() {
  lcdPrint(0, 0, "Control power       ");
}

void printManualPause() {
  lcdPrint(0, 0, "    -- PAUSE --     ");
}

//-----------------------------------------------------------------------------/
// Manual functions
//-----------------------------------------------------------------------------/

unsigned long executePause() {
  unsigned long pauseInit = millis();
  int readInput;
  printManualPause();
  while(true) {
    readInput = isPressedOneSecond(1);
    if (readInput > 0) break;
  }
  return millis() - pauseInit;
}

void executeManualMash() {
  unsigned long initTime = millis();
  unsigned long initStep = initTime;
  unsigned long now;
  int readInput;
  float myTemp;

  myPIDStartStep(TEMP_DEFAULT);

  printStep();
  printManualMash();
  printObjectiveTemp(TEMP_DEFAULT);
  printMashPower();

  while(true) {
    // Updating temperature functions
    myTemp = thermometerRead();
    printActualTemp(myTemp);

    // Updating time functions
    now = millis();
    printStepTimeD(now - initStep);
    printTotalTimeD(now - initTime);

    // Running PID
    myPIDLoop();
    myPIDUpdateFrontend();
    printMashPower();

    // Checking inputs
    // Has the pause button been pressed?
    readInput = isPressedOneSecond(1);
    if (readInput > 0) {
      now = executePause();
      printManualMash();
      printObjectiveTemp(setpoint);
      initTime += now;
      initStep += now;
    }
    // Do we have a step reset or a full reset?
    readInput = isPressedOneSecond(2);
    if (readInput == 1) initStep = millis();
    else if (readInput == 2){
      initStep = millis();
      initTime = millis();
    }
    // Has someone changed the objective temperature?
    readInput = readEncoder();
    if (readInput != 0) {
      // setpoint is a PID variable storing the objective temperature
      setpoint += readInput;
      setpoint = constrain(setpoint,0,MASH_TEMP_MAX);
      printObjectiveTemp(setpoint);
    }
    delay(250); // No need to check the PID loop too often
  }
}

void executeManualBoil() {
  unsigned long initTime = millis();
  unsigned long initStep = initTime;
  unsigned long now;
  int readInput;
  float myTemp;

  resistancesStartStep();

  printStep();
  printManualBoil();
  printMashPower();

  while(true) {
    // Updating temperature functions
    myTemp = thermometerRead();
    printActualTemp(myTemp);

    // Updating time functions
    now = millis();
    printStepTimeD(now - initStep);
    printTotalTimeD(now - initTime);

    // Running resistances
    resistancesLoop();
    myPIDUpdateFrontend();
    printBoilPower(resistancesPower);

    // Checking inputs
    // Has the pause button been pressed?
    readInput = isPressedOneSecond(1);
    if (readInput > 0) {
      now = executePause();
      printManualBoil();
      initTime += now;
      initStep += now;
    }
    // Do we have a step reset or a full reset?
    readInput = isPressedOneSecond(2);
    if (readInput == 1) initStep = millis();
    else if (readInput == 2){
      initStep = millis();
      initTime = millis();
    }
    // Check if there has been any change by the user in the power it wants
    // to flow through the resistances.
    readInput = readEncoder();
    if (readInput != 0) {
      resistancesPower += readInput;
      resistancesPower = constrain(resistancesPower,0,20);
    }
    delay(250); // No need to check the PID loop too often
  }
}
