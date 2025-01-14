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
 *  DONE - Add battery voltage indicator 
 */

#include "HX711.h"
#include <Wire.h>
#include "grove_alphanumeric_display.h"

Seeed_Digital_Tube tube;
HX711 scale;

// Pin configuration
const int LOADCELL_DOUT_PIN = 2; // DT pin on HX711
const int LOADCELL_SCK_PIN = 3; // SCK pin on HX711
const int powerLatch = 4;  // Power latching digital pin
const int offButton = 5;   // Off button digial pin
const int tareButton = 6;  // Tare button digital pin
const int batteryLevelPin = A0; // 3.3v pin connected to analogue pin. 
  /* Since the 3.3v supply is constant within the range of interest and the analgue value is a ratio of voltage at the analogue pin / 
   * Vsupply the 3.3v supply as read at the analogue pin will be a ratio of voltage supply.
   * Since any Vin >= approx. 6.1v and within the input range of the Arduino will give a supply voltage of 5v, this
   * relationship is only valid below this threshold. However, with a 9v PP3 supplying the circuit, the board continuing to 
   * operate down to <5v and the voltage dispayed for a low battery warning only needing to be approximate, this method is adequate and 
   * negates the need for a voltage divider and potential increase in drain on the battery.
   */
  
  
// Configuration values
const int maxReading = 1580;  // Maximum reading to display. Values in excess of this will display an error message
const float batWarnLevel = 6000; // Voltage level at which low battery warning will be triggered. 
//  This must be <= 6000mV as the value read on the analogue pin will not vary above that value
const int noiseThresh = 5; // Minimum load change to be regarded as active scale usage
const int pwOffTime = 30000;  // Time after which autopower off will be activated after last recorded activity (milliseconds)

int reading;
int lastReading;
int tareVal;

void setup() {
  Serial.begin(115200);
  float batLevel; // Battery voltage level
  
  // Latch circuit
  pinMode(powerLatch, OUTPUT);     // Define latch circuit connection pin as an OUTPUT
  digitalWrite(powerLatch, HIGH);  // Keeps the circuit on

  // Setup display
  tube.setTubeType(TYPE_4, TYPE_4_DEFAULT_I2C_ADDR);
  tube.setBrightness(8); // Set dispaly brightness (1-15)
  tube.setBlinkRate(BLINK_OFF);
  
  // Set buttons to input
  pinMode(offButton, INPUT);       // Define power off connection pin as an INPUT
  pinMode(tareButton, INPUT);      // Define tare pin as an INPUT

  // Check for low battery
  batLevel = analogRead(batteryLevelPin);  // read the input voltage level
  batLevel = round((1715.87797993738 - batLevel) / 0.168057273524759);
//    Correlation between read analogue value and voltage read by DVM
  /* Data not sent to serial monitor to improve responsivness. Can be reinstated for debugging
  Serial.print("Battery level = ");
  Serial.print(batLevel);
  Serial.println("mV");
  */
  if (batLevel < batWarnLevel){
    lowBatteryWarning(batLevel);  
  }
  
  // Setup scales
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(64); // Set gain
  scale.set_scale(5274.f);  // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.set_median_mode(); // Use median of 3 values to reduce noise
  tareVal = read_tare();  // Read initial tare value
}

void loop() {
  unsigned long lastUseTime; // time in system clock time of last change in load reading above threshold
 if (scale.wait_ready_timeout(200)) {
    reading = round(scale.get_units(3)); // read scaled value but without offset- median of 3 values
    if (reading != lastReading){
      if (reading - tareVal > maxReading) { // Scale overload
        tube.displayString("err"); 
      }
      else {
        tube.clearBuf();
        tube.displayNum(reading);
        /*
        Serial.print("Scale reading: ");
        Serial.println(reading); 
        */
        if (reading < 0){ // Add the minus sign
          tube.setTubeSegments(FIRST_TUBE, SEGMENT_MIDDLE_LEFT + SEGMENT_MIDDLE_RIGHT);
          tube.display();
        }
      }
      if (abs(reading - lastReading) > noiseThresh) { // If activity above threshold is recorded reset the last use time
        lastUseTime = (millis());
        /*
        Serial.print("Power off timer reset: scale will auto power off in ");
        Serial.print(pwOffTime / 1000);
        Serial.println(" seconds");
        */
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
void lowBatteryWarning(int val){
  int startt; // Time of start of loop
//  for (int i = 0; i <= 2; i++){
    tube.displayString("low battery", 50);
    tube.clearBuf();  
    tube.setBlinkRate(BLINK_2HZ);
    tube.displayNum(int(val)); 
    startt = millis();
    while (millis() < startt + 2000){ // Keep checking for button press during voltage display delay. 
      // This gives more chance for the button press to be detected
      if (digitalRead(tareButton) == HIGH) { // Tare button to escape warning display
        tube.setBlinkRate(BLINK_OFF);  
        tube.displayString("ok"); 
        return(NULL);
      }  
    }
    tube.setBlinkRate(BLINK_OFF);  
//  }
}

// Read the tare value
float read_tare(){  // reset the scale to 0
  Serial.println("Read tare"); 
  tube.displayString("tare"); // Notify getting tare
  lastReading = -9999999;  // Call null reading so will always intialise
  scale.tare(); // measure tare value
  return(scale.get_tare());
}              

// set latch output to LOW to power off Arduino
void pwrOff() {
  Serial.println("Power off"); 
  tube.displayString("power off", 100);
  digitalWrite(powerLatch, LOW);
  while (1);  // don't do anything else
}

// -- End of file --
