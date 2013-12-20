
/*
 *  Boil funcions
 */

// Enable line below for debugging purposes
//#define DEBUG

const int BOIL_TIME_MAX = 180;  // Maximum time accepted

struct boilStep {
  int stepID;             // ID of step, usually its position within the profile
  int minutes;            // Duration of the boil in minutes
  boilStep *nextStep;
  boilStep *previousStep;
};

// Link to the beginning of step links
boilStep *firstBoilStep = NULL;
boilStep *lastBoilStep = NULL;

// Returns an empty pointer if there is an error
struct boilStep* boilNewStep(int minutes) {
  boilStep *myStep;
  
  // Verify inputs are within range
  if ((minutes+boilTimeStep(firstBoilStep)) > BOIL_TIME_MAX) return NULL;

  // Allocate memory to the new step structure
  myStep = (boilStep*) malloc(sizeof(struct boilStep));
  if (!myStep) return NULL;
  
  myStep->stepID = boilNbStep() + 1;  // Set the ID as total nb of steps + 1
  myStep->minutes = minutes;
  
  // Insert the new boilStep in the array
  myStep->previousStep = lastBoilStep;
  myStep->nextStep = NULL;
  if (firstBoilStep == NULL) firstBoilStep = myStep;
  if (lastBoilStep != NULL) lastBoilStep->nextStep = myStep;
  lastBoilStep = myStep;

  return myStep;
}

// Returns -1 if there is an error, 0 otherwise
int boilModifyStep(int minutes, boilStep *myStep) {
  // Verify inputs are within range
  if ((minutes+boilTimeStep(firstBoilStep)-myStep->minutes) > BOIL_TIME_MAX) return -1;  
  myStep->minutes = minutes;
  return 0;
}

// Returns -1 if there is an error, 0 otherwise
void boilDeleteStep(boilStep *myStep, boolean adjustId) {
  // If the Step is the last of the list
  if (boilNbStep() == 1) {
    firstBoilStep = NULL;
    lastBoilStep = NULL;
  } else {
    if (myStep == firstBoilStep) {
      firstBoilStep = myStep->nextStep;
      myStep->nextStep->previousStep = NULL;
    } else {
      myStep->nextStep->previousStep = myStep->previousStep;
    }
  
    if (myStep == lastBoilStep) {
      lastBoilStep = myStep->previousStep;
      myStep->previousStep->nextStep = NULL;
    } else {
      myStep->previousStep->nextStep = myStep->nextStep;  
    }
  }

  // Adjust IDs of remaining steps
  if (adjustId) {
    boilStep *thisStep=myStep;
    while (thisStep->nextStep != NULL) {
      thisStep->nextStep->stepID -= 1;
      thisStep=thisStep->nextStep;
    }
  }
  
  // Free the memory
  free(myStep);
}

// Returns the number of steps
int boilNbStep() {
  boilStep *thisStep = firstBoilStep;
  int nbSteps = 0;
  
  while (thisStep != NULL) {
    nbSteps++;
    thisStep = thisStep->nextStep;
  }
  return nbSteps;
}

// Returns boil total time in minutes
int boilTimeStep(boilStep *myStep) {
  boilStep *thisStep = myStep;
  int totalTime = 0;
  
  while (thisStep != NULL) {
    totalTime += thisStep->minutes;
    thisStep = thisStep->nextStep;
  }
  return totalTime;
}

void boilPrintSteps() {
  boilStep *thisStep = firstBoilStep;
  Serial.println("### Boil steps:");
  while (thisStep != NULL) {
    Serial.print("ID: ");
    Serial.print(thisStep->stepID);
    Serial.print(", time: ");
    Serial.println(thisStep->minutes);
    thisStep = thisStep->nextStep;
  }
}
