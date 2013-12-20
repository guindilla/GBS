/*
 *  Step funcions
 */

// Enable line below for debugging purposes
//#define DEBUG

const int MASH_TIME_MAX = 180;  // Maximum time accepted
const int MASH_TEMP_MAX = 99;  // Maximum temperature accepted

struct mashStep {
  int stepID;             // ID of step, usually its position within the profile
  int temperature;        // Temperature of the step
  int minutes;            // Duration of the step in minutes
  mashStep *nextStep;
  mashStep *previousStep;
};

// Link to the beginning of step links
mashStep *firstMashStep = NULL;
mashStep *lastMashStep = NULL;

// Returns an empty pointer if there is an error
struct mashStep* mashNewStep(int temperature, int minutes) {
  mashStep *myStep;
  
  // Verify inputs are within range
  if (temperature > MASH_TEMP_MAX ||
    (minutes+mashTimeStep(firstMashStep)) > MASH_TIME_MAX)
    return NULL;

  // Allocate memory to the new step structure
  myStep = (mashStep*) malloc(sizeof(struct mashStep));
  if (!myStep) return NULL;

  myStep->stepID = mashNbStep() + 1;// Set the ID as ID of last step + 1
  myStep->temperature = temperature;
  myStep->minutes = minutes;
  
  // Insert the new mashStep in the array
  myStep->previousStep = lastMashStep;
  myStep->nextStep = NULL;
  if (firstMashStep == NULL) firstMashStep = myStep;
  if (lastMashStep != NULL) lastMashStep->nextStep = myStep;
  lastMashStep = myStep;
  
  return myStep;
}

// Returns -1 if there is an error, 0 otherwise
int mashModifyStep(int temperature, int minutes, mashStep *myStep) {
  // Verify inputs are within range
  if (temperature > MASH_TEMP_MAX ||
    (minutes+mashTimeStep(firstMashStep)-myStep->minutes) > MASH_TIME_MAX)
    return -1;  
  myStep->temperature = temperature;
  myStep->minutes = minutes;
  return 0;
}

// Returns -1 if there is an error, 0 otherwise
void mashDeleteStep(mashStep *myStep, boolean adjustId) {
  // If the Step is the last of the list
  if (mashNbStep() == 1) {
    firstMashStep = NULL;
    lastMashStep = NULL;
  } else {
    if (myStep == firstMashStep) {
      firstMashStep = myStep->nextStep;
      myStep->nextStep->previousStep = NULL;
    } else {
      myStep->nextStep->previousStep = myStep->previousStep;
    }
  
    if (myStep == lastMashStep) {
      lastMashStep = myStep->previousStep;
      myStep->previousStep->nextStep = NULL;
    } else {
      myStep->previousStep->nextStep = myStep->nextStep;  
    }
  }

  // Adjust IDs of remaining steps
  if (adjustId) {
    mashStep *thisStep=myStep;
    while (thisStep->nextStep != NULL) {
      thisStep->nextStep->stepID -= 1;
      thisStep=thisStep->nextStep;
    }
  }

  // Free the memory
  free(myStep);
}


// Returns the number of steps otherwise
int mashNbStep() {
  mashStep *thisStep = firstMashStep;
  int nbSteps = 0;
  
  while (thisStep != NULL) {
    nbSteps++;
    thisStep = thisStep->nextStep;
  }
  return nbSteps;
}

// Returns mash total time in minutes
int mashTimeStep(mashStep *myStep) {
  mashStep *thisStep = myStep;
  int totalTime = 0;
  
  while (thisStep != NULL) {
    totalTime += thisStep->minutes;
    thisStep = thisStep->nextStep;
  }
  return totalTime;
}

void mashPrintSteps() {
  mashStep *thisStep = firstMashStep;
  Serial.println("### Mash steps:");
  while (thisStep != NULL) {
    Serial.print("ID: ");
    Serial.print(thisStep->stepID);
    Serial.print(", time: ");
    Serial.print(thisStep->minutes);
    Serial.print(", temperature: ");
    Serial.print(thisStep->temperature);
    Serial.println(" C");
    thisStep = thisStep->nextStep;
  }
}
