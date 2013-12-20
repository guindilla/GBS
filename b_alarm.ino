/*
 *  Alarm funcions
 */

// Tracks the activation of the alarm
boolean alarmActivated = false;
int alarmBlinking = LOW;

void alarmSetup() {
  pinMode(alarmPin, OUTPUT);
  digitalWrite(alarmPin,LOW);
}

void alarmOn() {
  alarmActivated = true;  
  digitalWrite(alarmPin, HIGH);
}

void alarmSoundOff() {
  // Switch-off the alarm sound
  digitalWrite(alarmPin, LOW);
}

void alarmOff() {
  alarmActivated = false;
  // Switch-off the alarm no matter its state
  digitalWrite(alarmPin, LOW);
}

boolean alarmIsActivated() {
  return alarmActivated;
}

void alarmBlink() {
  if (alarmActivated == true) {
    if (alarmBlinking == HIGH) {
      alarmBlinking = LOW;
    } else {
      alarmBlinking = HIGH ;
    }
    // Insert here the code executed when the alarm is on (sound, light, etc.)
    digitalWrite(alarmPin, alarmBlinking);
  }
}

// Interval of alarm blinking
const int blinking = 1000;

void alarm() {
  int readInput;
  unsigned long blinkWindow = millis();
  bool sound = true;

  lcdPrint(3, 0, " Next |No snd| N.a. ");

  alarmOn();

  while (true) {
    readInput = isPressedOneSecond(1);
    if (readInput > 0) break; // Exits the while loop and ends the function
    if (millis() - blinkWindow > blinking && sound) {
      blinkWindow = millis();
      alarmBlink();
    }
    readInput = isPressedOneSecond(2);
    if (readInput > 0){
      alarmSoundOff(); // Stops the sound of the alarm
      sound = false;
    }
  }
  alarmOff();
}

void alarmPrint(const char* message) {
  lcdClear();
  lcdPrint(1, 0, message);
  alarm();
}

void alarmPrintTime(const char* message, int number) {
  lcdClear();
  lcdPrint(1, 0, message);
  lcd.print(": ");
  lcdTens(number);
  lcd.print(" m");
  alarm();
}
