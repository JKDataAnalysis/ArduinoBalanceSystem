# ArduinoBalanceSystem
Development of a force instrumented system for balance feedback using Arduinos. The system is intended for use by physiotherapists working with stroke rehabilitation patients. Strokes can often affect balance awareness leading patients being unable to correct unbalanced postures. Whilst, a physiotherapist can provide postural feedback to the patient when they are present, this system will provide balance feedback to patients allowing them to practice independently and potentially speeding theie recovery process.

The development of this system will be conducted in two phases. The first stage of development is to use a simpler system to develop and test the basic subsystems of the system including the required code base. This will be based on a simple set of kitchen scales using the same instrumentation, feedback and power systems.

Part 1: kitchen scales
The components used in this prototype that will be used in the subsequent balance system are:
  * Force measurement: HX711 24 bit ADC connected to a load cell. The balance system will require two load cells and ADCs but the basic configuration will be the same.
  * Visual display: feedback to use with a display using the i2c protocol.
  * Latching power circuit: this will allow the system to auto-poweroff if it is not used for extended periods. In order to maximise portability, the system will need to be battery powered. Auto power off will avoid batteries becoming discharged if the system is not shut down between uses.
  * Low battery warning: Again, this will help to ensure that the system is ready for usage.

Part 2: Development of basic balance feedback system
There are several challenges that will need to be addressed for the balance system that were not required for the kitchen scales;
1) Dual load cells: This is not expected to be a significant issue at the relatively low sample rates required by the system. Since the system is intended only for standing balance, the basic sample rate of the HX711 ADCs (10Hz) is expected to be adequate so any temporal misalignment in sampling the two load cells or processing load on the Arduino is not expected to be significant.
2) Provision of feedback: This will be a crucial area for devlopment for this system to be effective. Initally the system will use just basic visual cues to a small screen but other feedback methods may be more appropriate. The positioning of the screen will also need to be considered to ensure correct head position. Feedback also needs to be clear enough for users who may have impaired vision.
3) Adjustable sensitivity: The balance correction feedback will need to be adjustable so that it can be made more sensitive as the patient progresses. This will include both varying sensitivity to the level of inequality in load distribution and the rate at which the feedback will update in response to change.
4) Since the system is intened for patient usage, it needs to be made simple to use to make it accessible for users with a range of technical competencies. It should also operate independent of reliance on other devices, e.g. smart phones. For the end user, the system will therefore simply have on/ off buttons. With sensitivity adjustments via a simple menu for the clinician.

Part 3: Enhancement of the system
This will be informed by user feedback but is expected to include
1) Enhanced feedback options, e.g. auditory
2) Recording and download of data so patient activity and progress can be monitored.
