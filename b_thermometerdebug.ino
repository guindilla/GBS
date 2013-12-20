/*
 *  Thermometer funcions
 */
// http://www.arduino.cc/playground/Learning/OneWire
// https://github.com/milesburton/Arduino-Temperature-Control-Library/blob/master/DallasTemperature.h
// http://www.strangeparty.com/2010/12/13/arduino-1-wire-temperature-sensors/
// Enable line below for debugging purposes

double thermometerTemperature = 20;
unsigned long thermometerLastRead = 0;
unsigned long thermometerReadEach = 1000; // miliseconds

void thermometerSetup()
{
//  input=20;
//  output=0;
//  setpoint=20;
}

float thermometerRead()
{
  // Use with Kp, Ki, Kd: XX, XX, XX
//  float degIncrease = 4;   // Max number of ºC that the temp increases if output is set at maximum  
//  float degDecrease = 1; // ºC that the temp decreases if output is set to 0  
  // Use with Kp, Ki, Kd: 2000, 0, 0
//  float degIncrease = 0.4;   // Max number of ºC that the temp increases if output is set at maximum  
//  float degDecrease = 0.1; // ºC that the temp decreases if output is set to 0  
  // Use with Kp, Ki, Kd: 450, 0, 0
//  float degIncrease = 0.021;   // Max number of ºC that the temp increases if output is set at maximum  
//  float degDecrease = 0.01; // ºC that the temp decreases if output is set to 0  
  // Use with Kp, Ki, Kd: ???, ???, ???
  // Calculated based on initial testings with my equipment (estimated at 0.917ºC per minute)
  float degIncrease = 0.03056;   // Max number of ºC that the temp increases if output is set at maximum  
  float degDecrease = degIncrease *0.4; // ºC that the temp decreases if output is set to 0
  unsigned long thermometerTime = millis();
  if (thermometerTime - thermometerLastRead > thermometerReadEach) {
    // The system could lose heat even if the resistance is on, as the resistance must provide
    // enough heat to compensate heat losses.
    if (output > MIN_ON_RESISTANCE)
      thermometerTemperature = thermometerTemperature +
                               output/windowSize * (degIncrease+degDecrease) -
                               degDecrease;
    else
      thermometerTemperature = thermometerTemperature - degDecrease;
      thermometerLastRead = thermometerTime;
  }
  thermometerTemperature = constrain(thermometerTemperature,0,99);
  input = thermometerTemperature;
  return (float) thermometerTemperature;
}
