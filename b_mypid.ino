/*
 *  PID funcions
 */

//-----------------------------------------------------------------------------/
// Common variables
//-----------------------------------------------------------------------------/

// input: The variable the program is trying to control
// output: The variable that will be adjusted by the pid
// setpoint: The objective value
double input, output, setpoint;

unsigned long windowStartTime;

//-----------------------------------------------------------------------------/
// Resistance functions
//-----------------------------------------------------------------------------/

// Controls the power transmitted to the resistances
int resistancesPower=0;

// Enables the resistances
void resistance1On() {
  digitalWrite(resistancePin1,HIGH);
}

void resistance2On() {
  if (resistancePin2 != 0) digitalWrite(resistancePin2,HIGH);
}

void resistancesOn() {
  resistance1On();
  resistance2On();
  resistancesPower=20;
}

// Disables the resistances
void resistance1Off() {
  digitalWrite(resistancePin1,LOW);
}

void resistance2Off() {
  if (resistancePin2 != 0) digitalWrite(resistancePin2,LOW);
}

void resistancesOff() {
  resistance1Off();
  resistance2Off();
  resistancesPower=0;
}

// Initialising resistances
void resistancesSetup(){
  pinMode(resistancePin1, OUTPUT);
  if (resistancePin2 != 0) {
    pinMode(resistancePin2, OUTPUT);
  }
  resistancesOff();
}

void resistancesStartStep(){
  windowStartTime = millis();
  setpoint = BOIL_TEMP;

  // Ensure BOIL_POWER is within limits
  resistancesPower = constrain(BOIL_POWER,0,20);
  output = map(resistancesPower, 0, 20, 0, windowSize);
}

void resistancesLoop(){
  unsigned long myTime = millis();
  input = thermometerRead();
  output = map(resistancesPower, 0, 20, 0, windowSize);

  // Time to shift the Relay Window
  if (myTime - windowStartTime > windowSize) windowStartTime += windowSize;
    
  // Turn the output pin on/off based on power settings
  // If there are more than 2 resistances they get on and off alternatively
  //
  // Ex: output = 1000
  // ms: 0000  0500  1000  1500  2000  2500  3000  3500  4000  4500
  // R1: ||||||||||||________________________________________________
  // R2: ______________________________||||||||||||__________________
  //
  // Ex: output = 4000
  // ms: 0000  0500  1000  1500  2000  2500  3000  3500  4000  4500
  // R1: ||||||||||||||||||||||||||||||||||||||||||||||||____________
  // R2: ||||||||||||||||||____________||||||||||||||||||||||||||||||
  if (output > MIN_ON_RESISTANCE) {
    // Checking resistance 1...
    if (myTime-windowStartTime < output) resistance1On();
    else resistance1Off();
    // .. and resistance 2 in case there is any
    if ((myTime-windowStartTime+windowSize/2)%windowSize < output) resistance2On();
    else resistance2Off();
  }
}

//-----------------------------------------------------------------------------/
// PID functions
//-----------------------------------------------------------------------------/

// PID librairies
#include <PID_v1.h>

PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

void myPIDSetup() {
  myPID.SetOutputLimits(0, windowSize);
}

void myPIDStartStep(int targetTemp) {
  windowStartTime = millis();
  setpoint = targetTemp;
  output = windowSize;
  // By default we start with no power
  resistancesPower = 0;
}

void myPIDStopStep() {
  myPID.SetMode(MANUAL);
  resistancesOff();
}

void myPIDLoop(){
  input = thermometerRead();
  unsigned long myTime = millis();
  boolean pidMode = myPID.GetMode();

  if (input > setpoint - PID_ACTIVATE_TEMP && !pidMode) myPID.SetMode(AUTOMATIC);  
  if (input < setpoint - PID_ACTIVATE_TEMP && pidMode) {
    myPID.SetMode(MANUAL);
    resistancesOn();    
  }
  
  if (myPID.GetMode()) {
    myPID.Compute();
    
    // Calculating power to pass to resistances based on output variable
    resistancesPower = map(output, 0, windowSize, 0, 20);
    
    // Time to shift the Relay Window
    if (myTime - windowStartTime > windowSize) windowStartTime += windowSize;
    
    // Turn the output pin on/off based on power settings
    // If there are more than 2 resistances they get on and off alternatively
    //
    // Ex: output = 1000
    // ms: 0000  0500  1000  1500  2000  2500  3000  3500  4000  4500
    // R1: ||||||||||||________________________________________________
    // R2: ______________________________||||||||||||__________________
    //
    // Ex: output = 4000
    // ms: 0000  0500  1000  1500  2000  2500  3000  3500  4000  4500
    // R1: ||||||||||||||||||||||||||||||||||||||||||||||||____________
    // R2: ||||||||||||||||||____________||||||||||||||||||||||||||||||
    if (output > MIN_ON_RESISTANCE) {
      // Checking resistance 1...
      if (myTime-windowStartTime < output) resistance1On();
      else resistance1Off();
      // .. and resistance 2 in case there is any
      if ((myTime-windowStartTime+windowSize/2)%windowSize < output) resistance2On();
      else resistance2Off();
    } else resistancesOff();
  } else resistancesOn();
}

//-----------------------------------------------------------------------------/
// PID frontend
//-----------------------------------------------------------------------------/

// Code taken from the PID frontend published by Brett Beauregard

//this will help us know when to talk with processing
unsigned long serialTime = 0;

union {                // This Data structure lets
  byte asBytes[24];    // us take the byte array
  float asFloat[6];    // sent from processing and
}                      // easily convert it to a
foo;                   // float array

// getting float values from processing into the arduino
// was no small task.  the way this program does it is
// as follows:
//  * a float takes up 4 bytes.  in processing, convert
//    the array of floats we want to send, into an array
//    of bytes.
//  * send the bytes to the arduino
//  * use a data structure known as a union to convert
//    the array of bytes back into an array of floats

//  the bytes coming from the arduino follow the following
//  format:
//  0: 0=Manual, 1=Auto, else = ? error ?
//  1: 0=Direct, 1=Reverse, else = ? error ?
//  2-5: float setpoint
//  6-9: float input
//  10-13: float output  
//  14-17: float P_Param
//  18-21: float I_Param
//  22-245: float D_Param
void SerialReceive()
{

  // read the bytes sent from Processing
  int index=0;
  byte Auto_Man = -1;
  byte Direct_Reverse = -1;
  while(Serial.available()&&index<26)
  {
    if(index==0) Auto_Man = Serial.read();
    else if(index==1) Direct_Reverse = Serial.read();
    else foo.asBytes[index-2] = Serial.read();
    index++;
  } 
  
  // if the information we got was in the correct format, 
  // read it into the system
  if(index==26 &&
    (Auto_Man==0 || Auto_Man==1) &&
    (Direct_Reverse==0 || Direct_Reverse==1))
  {
    setpoint=double(foo.asFloat[0]);
    //Input=double(foo.asFloat[1]);       // * the user has the ability to send
                                          //   the value of "Input"  in most
                                          //   cases (as in this one) this is
                                          //   not needed.
    if(Auto_Man==0)                       // * only change the output if we are
    {                                     //   in manual mode. Otherwise we'll
      output=double(foo.asFloat[2]);      //   get an output blip, then the
    }                                     //   controller will overwrite.
    
    double p, i, d;                       // * read in and set the controller
    p = double(foo.asFloat[3]);           //   tunings
    i = double(foo.asFloat[4]);           //
    d = double(foo.asFloat[5]);           //
    myPID.SetTunings(p, i, d);            //
    
    if(Auto_Man==0) myPID.SetMode(MANUAL); // * set the controller mode
    else myPID.SetMode(AUTOMATIC);

    if(Direct_Reverse==0)
      myPID.SetControllerDirection(DIRECT); // * set the controller Direction
    else myPID.SetControllerDirection(REVERSE);
  }
  Serial.flush(); // * clear any random data from the serial buffer
}

// Unlike our tiny microprocessor, the processing ap
// has no problem converting strings into floats, so
// we can just send strings.  much easier than getting
// floats from processing to here no?
void SerialSend()
{
  Serial.print("PID ");
  Serial.print(setpoint);   
  Serial.print(" ");
  Serial.print(input);   
  Serial.print(" ");
  Serial.print(output);   
  Serial.print(" ");
  Serial.print(myPID.GetKp());   
  Serial.print(" ");
  Serial.print(myPID.GetKi());   
  Serial.print(" ");
  Serial.print(myPID.GetKd());   
  Serial.print(" ");
  if(myPID.GetMode()==AUTOMATIC) Serial.print("Automatic");
  else Serial.print("Manual");  
  Serial.print(" ");
  if(myPID.GetDirection()==DIRECT) Serial.println("Direct");
  else Serial.println("Reverse");
}

void myPIDUpdateFrontend() {
  //send-receive with processing if it's time
  if(millis()>serialTime)
  {
    SerialReceive();
    SerialSend();
    serialTime+=500;
  }
}
