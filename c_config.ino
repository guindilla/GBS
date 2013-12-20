/*
 *  System configuration
 */

//-----------------------------------------------------------------------------/
// Mash print functions
//-----------------------------------------------------------------------------/

// Update mash step number, use only with numbers < 100
void printConfigMashStep(int step) {
  lcdCursor(0, 10);
  lcdTens(step);
}

// Update mash temperature
void printConfigMashTemp(int temp) {
  lcdCursor(1, 6);
  lcdTens(temp);
}

// Update mash time
void printConfigMashTime(int time) {
  lcdCursor(2, 6);
  lcdHundreds(time);
}

// Shows which field is being modified
const int timeField = 0;
const int tempField = 1;
void printSelectedField(int field) {
  if (field == tempField) {
    lcdPrint(1, 11, "<--");
    lcdPrint(2, 14, "   ");
  } else if (field == timeField) {
    lcdPrint(1, 11, "   ");
    lcdPrint(2, 14, "<--");
  }
}

// mash step baseline
void printConfigMash() {
  lcdPrint(0, 0, "Mash step           ");
  lcdPrint(1, 0, "Temp:    C          ");
  lcdPrint(2, 0, "Time:     min       ");
  lcdPrint(3, 0, "New/End | Nxt | +/- ");
}

//-----------------------------------------------------------------------------/
// Mash configuration function
//-----------------------------------------------------------------------------/

// Button 1: Next step / end mash configuration
// Button 2: Change variable (time/temperature)
// Rotary switch: Increase/decrease variable (time/temperature)
void configureMash() {
  int currentTemp = TEMP_DEFAULT;
  int currentTime;
  int currentStep;
  int readInput;
  int selectedField;
  mashStep *modifiedStep = NULL;

  lcdClear();
  printConfigMash();
  while (true) {
    delay(500);
    currentStep = mashNbStep() + 1;
    currentTime = TIME_DEFAULT;
    selectedField = tempField;
    printConfigMashStep(currentStep);
    mashStep *modifiedStep = NULL;
    
    while (true) {
      printConfigMashTemp(currentTemp);  
      printConfigMashTime(currentTime);
      printSelectedField(selectedField);

      readInput = isPressedOneSecond(1);
      if (readInput == 1) {
        // End mash step if button pressed for less than 1 second
        if (modifiedStep == NULL) mashNewStep(currentTemp,currentTime);
        else mashModifyStep(currentTemp,currentTime, modifiedStep);
        break;
      } else if (readInput == 2) {
        // End mash configuration if button pressed for more than 1 second
        if (modifiedStep == NULL) mashNewStep(currentTemp,currentTime);
        else mashModifyStep(currentTemp,currentTime, modifiedStep);
        #ifdef M_DEBUG
          mashPrintSteps();
        #endif
        return;
      }
      readInput = isPressedOneSecond(2); 
      if (readInput == 1) {
        // Trick to select a different field, as their value are 0 and 1
        selectedField = !selectedField;
        printSelectedField(selectedField);
        delay(500);
      } else if (readInput == 2) {
        // Are we modifying a previous step already?
        if (modifiedStep == NULL) {
          if (lastMashStep != NULL) modifiedStep = lastMashStep;
        } else {
          if (modifiedStep->previousStep != NULL) modifiedStep = modifiedStep->previousStep;
        }
        // If we have a modifiedStep, print the updated information
        if (modifiedStep != NULL) {
            currentStep = modifiedStep->stepID;
            currentTime = modifiedStep->minutes;
            currentTemp = modifiedStep->temperature;
            printConfigMashStep(currentStep);
            printConfigMashTemp(currentTemp);
            printConfigMashTime(currentTime);
        }
      }
      readInput = readEncoder();
      if (selectedField == tempField) {
        currentTemp += readInput;
        currentTemp = constrain(currentTemp, 0, MASH_TEMP_MAX);
        printConfigMashTemp(currentTemp);
      } else if (selectedField == timeField) {
        currentTime += readInput;
        currentTime = constrain(currentTime, 1, MASH_TIME_MAX);
        printConfigMashTime(currentTime);
      }
    }
  }
}

//-----------------------------------------------------------------------------/
// Boil print functions
//-----------------------------------------------------------------------------/

// Update boil step number, use only with numbers < 100
void printConfigBoilStep(int step) {
  lcdCursor(0, 10);
  lcdTens(step);
}

// Update boil time
void printConfigBoilTime(int time) {
  lcdCursor(1, 6);
  lcdHundreds(time);
}

// Update boil total time
void printConfigBoilTotalTime(int time) {
  lcdCursor(2, 12);
  lcdHundreds(time);
}

// Boil step baseline
void printConfigBoil() {
  lcdPrint(0, 0, "Boil step           ");
  lcdPrint(1, 0, "Time:     min       ");
  lcdPrint(2, 0, "Total time:     min ");
  lcdPrint(3, 0, "New/End | N.a. | +/-");
}

//-----------------------------------------------------------------------------/
// Boil configuration function
//-----------------------------------------------------------------------------/

// Button 1: Next step / end boil configuration
// Button 2: N.a.
// Rotary switch: Increase/decrease time
void configureBoil() {
  int currentTime;
  int currentStep;
  int totalTime = 0;
  int readInput;
  boilStep *modifiedStep = NULL;
  
  lcdClear();
  printConfigBoil();
  while (true) {
    delay(500);
    currentStep = boilNbStep() + 1;
    currentTime = TIME_DEFAULT;
    printConfigBoilStep(currentStep);
    totalTime = boilTimeStep(firstBoilStep);

    while (true) {
      printConfigBoilTime(currentTime);
      printConfigBoilTotalTime(totalTime + currentTime);

      readInput = isPressedOneSecond(1);
      if (readInput == 1) {
        // End boil step if button pressed for less than 1 second
        if (modifiedStep == NULL) boilNewStep(currentTime);
        else boilModifyStep(currentTime, modifiedStep);
        break;
      } else if (readInput == 2) {
        // End boil configuration if button pressed for more than 1 second
        if (modifiedStep == NULL) boilNewStep(currentTime);
        else boilModifyStep(currentTime, modifiedStep);
        #ifdef M_DEBUG
          boilPrintSteps();
        #endif
        return;
      }
      readInput = isPressedOneSecond(2); // Nothing to do if button 2 is pressed
      if (readInput == 2) {
        // Are we modifying a previous step already?
        if (modifiedStep == NULL) {
          if (lastBoilStep != NULL) modifiedStep = lastBoilStep;
        } else {
          if (modifiedStep->previousStep != NULL) modifiedStep = modifiedStep->previousStep;
        }
        // If we have a modifiedStep, print the updated information
        if (modifiedStep != NULL) {
            currentStep = modifiedStep->stepID;
            currentTime = modifiedStep->minutes;
            printConfigBoilStep(currentStep);
            printConfigBoilTime(currentTime);
        }
      }
      readInput = readEncoder();
      currentTime += readInput;
      if (currentTime < 1) currentTime = 1;
      if (currentTime > BOIL_TIME_MAX) currentTime = BOIL_TIME_MAX;
      if (totalTime + currentTime > 999) totalTime = 999 - currentTime;
      printConfigBoilTime(currentTime);
      printConfigBoilTotalTime(totalTime + currentTime);
    }
  }
}
