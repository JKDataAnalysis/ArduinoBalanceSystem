/* KNOWN BUGS
 *  > If a negative readings less than -999 is recorded then the screen will scroll The maximum value that will need to 
 *  be displayed by the left most digit is one so the left part of the horizontal bar could be used. The first digit would need to be stripped
 *  out and replaced with a combined -1. Given that, this is unlikely to be an issue in practice, it is probably not worth doing
 *  
 */
/* DEVELOPMENTS STILL NEEDED
 *  DONE - Make modifications required for new display
 *  DONE - Add auto-power off function
 *  DONE - Add overload error message
 *  DONE - Add battery voltage indicator - 
 *    - Voltage divider to bring a safe ratio of the battery voltage onto an analogue pin, then read that at suitable points
 *    ' A 1:1 divider with 18k resistors will reduce 10V to 5V and have an impedance less than 10k which will ensure accurate ADC readings.'
 */

#include "HX711.h"
#include <Wire.h>
#include "grove_alphanumeric_display.h"

Seeed_Digital_Tube tube;
HX711 scale;

const int LOADCELL_DOUT_PIN = 2; // DT pin on HX711
const int LOADCELL_SCK_PIN = 3; // SCK pin on HX711
const int powerLatch = 4;  // Power latching on Digital pin 5
const int offButton = 5;   // Off button
const int tareButton = 6;  // Tare button
const int batteryLevelPin = A3; // voltage divider (1:1) connected across Vin and ground to analog pin 3
const int maxReading = 1580;  // Maximum reading to display. Values in excess of this will display an error message
const float batWarnLevel = 6.5; // Voltage level at which low battery warning will be triggered 
const int noiseThresh = 5; // Minimum load change to be regarded as active scale usage
const int pwOffTime = 30000;  // Time after which autopower off will be activated after last recorded activity (milliseconds)

int reading;
int lastReading;
int tareVal;

void setup() {
  Serial.begin(38400);
  float batLevel; // Battery voltage level

  // Setup display
  tube.setTubeType(TYPE_4, TYPE_4_DEFAULT_I2C_ADDR);
  tube.setBrightness(8); // Set dispaly brightness (1-15)
  tube.setBlinkRate(BLINK_OFF);
  
  // Latch circuit
  pinMode(powerLatch, OUTPUT);     // Define latch circuit connection pin as an OUTPUT
  digitalWrite(powerLatch, HIGH);  // Keeps the circuit on
  pinMode(offButton, INPUT);       // Define power off connection pin as an INPUT
  pinMode(tareButton, INPUT);      // Define tare pin as an INPUT

  // Check for low battery
  batLevel = analogRead(batteryLevelPin);  // read the input voltage level
  batLevel = batLevel / 1023 * 4.65 * 2;
    // [1023] Full range value of analogue readings
    // [4.65] Ref voltage is nominally 5v but reading from DVM is 4.65v
    // [2] Voltage divider is 2 18k resistors so voltage to analogue pin is half Vin

  if (batLevel < batWarnLevel){
    lowBatteryWarning(batLevel);  
  }
  
  // Setup scales
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(64); // Set gain
  scale.set_scale(5274.f);  // this value is obtained by calibrating the scale with known weights; see the README for details
  tareVal = read_tare();
}

void loop() {
  unsigned long lastUseTime; // time in system clock time of last change in load reading above threshold
 if (scale.wait_ready_timeout(200)) {
    reading = round(scale.get_units()); // read scaled value but without offset
    if (reading != lastReading){
      if (reading - tareVal > maxReading) { // Scale overload
        tube.displayString("err"); 
      }
      else {
        tube.clearBuf();
        tube.displayNum(reading);
        if (reading < 0){ // Add the minus sign
          tube.setTubeSegments(FIRST_TUBE, SEGMENT_MIDDLE_LEFT + SEGMENT_MIDDLE_RIGHT);
          tube.display();
        }
      }
      if (abs(reading - lastReading) > noiseThresh) { // If activity above threshold is recorded reset the last use time
        lastUseTime = (millis());
        Serial.println("Power off timer reset");
      }
    }
    lastReading = reading;

    if (digitalRead(tareButton) == HIGH) { // Call tare function
      tareVal = read_tare();
    }
    
    if (digitalRead(offButton) == HIGH || millis() > lastUseTime + pwOffTime) { // call shutdown on pwroff button press or timeout
      pwrOff();
    }
  }
  else {
    Serial.println("HX711 not found.");
  }
}


// Display low input voltage level warning
void lowBatteryWarning(float val){
  int startt; // Time of start of loop
  for (int i = 0; i <= 2; i++){
    tube.displayString("low battery", 50);
    tube.clearBuf();  
    tube.setBlinkRate(BLINK_2HZ);
    tube.displayNum(int(val * 100)); // * 100 to scale value to dislay so that integer value is to left of decimal point
      // will always be < 10v and, even if it wasn't, it wouldn't be getting called as a low voltage
    tube.setPoint(false,true);
    tube.display();
    startt = millis();
    while (millis() < startt + 1000){ // Keep checking for button press during voltage display delay. 
      // This gives more chance for the button press to be detected
      if (digitalRead(tareButton) == HIGH) { // Tare button to escape warning display
        tube.setBlinkRate(BLINK_OFF);  
        tube.displayString("ok"); 
        return(NULL);
      }  
    }
    tube.setBlinkRate(BLINK_OFF);  
  }
}

// Read the tare value
float read_tare(){  // reset the scale to 0
  tube.displayString("tare"); // Notify getting tare
  lastReading = -9999999;  // Call null reading so will always intialise
  scale.tare(); // measure tare value
  return(scale.get_tare());
}              

// set latch output to LOW to power off Arduino
void pwrOff() {
  tube.displayString("power off", 100);
  digitalWrite(powerLatch, LOW);
  while (1);  // don't do anything else
}
