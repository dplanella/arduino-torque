#include <SoftwareSerial.h>
 
/**
 * Torque for Android Arduino sample program by Ian Hawkins <ian@torque-bhp.com> http://torque-bhp.com/
 * You will require Torque Pro 1.8.82 or newer
 * 
 * Setup the bluetooth device as an OBD2 adapter in Torque.  Torque will automatically recognise the Arduino and will
 * import sensors automatically based on the configuration settings below.
 * 
 * This code is released under the LGPL v2 
 * 
 * This has been tested on:
 * - Arduino UNO
 * - Arduino Nano
 * 
 * Connect pins 2 and 3 to your bluetooth device's rx and tx pins (2 to txd on bluetooth device, 3 to rxd)
 * 
 * This code allows you to gather sensors from the arduino to display in the app. It can be extended to whatever you want.
 */
 
// Various constants.
const String ATE = "ATE"; // Echo off/on
const String ATI = "ATI"; // Version id
const String ATZ = "ATZ"; // Reset
const String ATS = "ATS"; // Set protocol X
const String ATH = "ATH"; // Headers off / on
const String ATL = "ATL"; // Linefeeds off/on
const String ATM = "ATM"; // Memory off/on
const String GETDEFINITIONS = "GETDEFINITIONS"; // Get sensor definitions
const String GETCONFIGURATION = "GETCONFIGURATION"; // Get config of app (hide car sensors, devices sensors, etc)
const String GETSENSORS = "G"; // Get sensor values, one shot.
const String SETSENSOR = "S"; // Set a sensor value
const String PROMPT = ">";
const String CANBUS = "6"; // canbus 500k 11 bit protocol id for elm.
const String ATDPN = "ATDPN";
const String ATDESC = "AT@1";
const String ATAT = "ATAT";
const String LF = "\n";
const String VERSION = "Torque Protocol Interface v0.0.1"; // Don't change this - it's used by Torque so it knows what interface it is connected to
const String VERSION_DESC = "Torque For Android Protocol Interface";
const String OK = "OK";
const String ANALOG = "a";
const String DIGITAL = "d";
const String IS_INPUT = "i";
const String IS_OUTPUT = "o";

// RX buffer
String fromTorque = "";
 
/**
 * Array of sensors we will advertise to Torque so it can automatically import them. Using strings
 * Stucture is:
 *  
 *  Arduino Pin, Arduino pin type, Input/Ouput, Default value(if output), ShortName, Long name,
 *      units, minimum value, maximum value
 *  
 *  Caveats:  Don't use a '>' in any of the names, 
 *            Update 'SENSORSCHANNELS' with the number of channels.
 *            Analog outputs are PWM on digital pins.
 *  
 */
const int SENSORSCHANNELS = 3;
const int SENSORSATTRIBUTES = 9;
const int SENSORSSIZE = SENSORSATTRIBUTES * SENSORSCHANNELS; // each line is 9 attributes, and we have 3 lines.
const String sensors[SENSORSSIZE] = {
                    "0", ANALOG,  IS_INPUT,   "-",  "Pot 1", "Potentiometer 1",  "v", "0", "5",
                    "1", ANALOG,  IS_INPUT,   "-",  "LDR 1", "Light Sensor 1",   "v", "0", "5",
                    "4", DIGITAL, IS_OUTPUT,  "0",  "Dout1", "Digital Out 1",  "bit", "0", "1"
                   }; 
/**
 * Configuration directives for the app to hide various things. Comma separated. Remove to enable visibility in Torque
 *  - handy if your project isn't car related or you want to make sensor selections relatively easy.
 *  
 *  Supported types:
 *    NO_CAR_SENSORS  - hide any car related sensors
 *    NO_DEVICE_SENSORS - hide any device (phone) sensors
 *    
 */
const String CONFIGURATION = "NO_CAR_SENSORS,NO_DEVICE_SENSORS"; 
 
// Setup bluetooth module on pins 2 and 3 (you can't use these digial pins in the sensor list or it'll break comms)
// Use pins 8, 9 on Arduino Micro
// Most other boards can use pins 2 and 3

const int rxPin = 2;
const int txPin = 3;

SoftwareSerial mySerial(rxPin, txPin);

const int BAUDRATE = 9600;
 
 
void setup() {
  // Init the pins 
  initSensors();
  Serial.begin(BAUDRATE);    // the GPRS baud rate 
  delay(600);
  mySerial.begin(BAUDRATE);              
 }
 
void loop() {
 
  /**
   * Grab data from the bluetooth module, parse it.
   */
  if (mySerial.available()) {
     char c = mySerial.read();
     // If end of command is detected, process the command buffer
     // and clear it afterwards
     if ((c == '\n' || c == '\r') && fromTorque.length() > 0) {
        fromTorque.toUpperCase();
        processCommand(fromTorque);
        fromTorque = "";
     // Otherwise, keep reading into the command buffer
     } else if (c != ' ' && c != '\n' && c !='\r') {
        // Ignore spaces.
        fromTorque += c; 
     }
  }
}
 
/**
 * Parse the commands sent from Torque
 */
void processCommand(String command) {
 
   // Debug - see what torque is sending on your serial monitor
   Serial.println(command);
 
   // Simple command processing from the app to the arduino..
   if (command.equals(ATZ)) {
       initSensors(); // reset the pins
       mySerial.print(VERSION);
       mySerial.print(LF); 
       mySerial.print(OK);
   } else if (command.startsWith(ATE)) {
       mySerial.print(OK); 
   } else if(command.startsWith(ATI)) {
       mySerial.print(VERSION);
       mySerial.print(LF);
       mySerial.print(OK);
   } else if (command.startsWith(ATDESC)) {
       mySerial.print(VERSION_DESC); 
       mySerial.print(LF);
       mySerial.print(OK);
   } else if (command.startsWith(ATL)) {
       mySerial.print(OK);
   } else if (command.startsWith(ATAT)) {
       mySerial.print(OK);
   } else if (command.startsWith(ATH)) {
       mySerial.print(OK);
   } else if (command.startsWith(ATM)) {
       mySerial.print(OK);
   } else if (command.startsWith(ATS)) {
       // Set protocol
       mySerial.print(OK);
   } else if (command.startsWith(ATDPN)) {
       mySerial.print(CANBUS);
   } else if (command.startsWith(GETDEFINITIONS)) {
       showSensorDefinitions();
   } else if (command.startsWith(GETSENSORS)) {
       getSensorValues();
   } else if (command.startsWith(GETCONFIGURATION)) {
       getConfiguration();
   } else if (command.startsWith(SETSENSOR)) {
       setSensorValue(command);
   }
 
   mySerial.print(LF); 
   mySerial.print(PROMPT);
 
 
}
 
/**
 * List all the sensors to the app
 */
void showSensorDefinitions() {
   int id = 0;
   for (int i = 0; i < SENSORSCHANNELS; i++) {
      for (int j = 0; j < SENSORSATTRIBUTES; j++) {
         id = (i * SENSORSATTRIBUTES) + j;
         mySerial.print(sensors[id]);
 
         if (id + 1 < SENSORSSIZE) {
            mySerial.print(',');
         }
      }
      mySerial.print(LF);
   }
}
 
/**
 * Dump sensor information for input sensors.
 * 
 * Format to Torque is id:type:value
 */
void getSensorValues() {
   for (int i = 0; i < SENSORSCHANNELS; i++) {
      int channel = i * SENSORSATTRIBUTES;
      int pinNum = sensors[channel].toInt();
      String pinType = sensors[channel + 1];
      boolean isOutput = sensors[channel + 2].equals(IS_OUTPUT);
 
      // Print input value
      if (!isOutput) {
         mySerial.print(pinNum);
         mySerial.print(":");
         mySerial.print(pinType);
         mySerial.print(":");
         if (pinType.equals(ANALOG)) {
            mySerial.print(analogRead(pinNum));
         } else if (pinType.equals(DIGITAL)) {
            mySerial.print(digitalRead(pinNum));
         }
         mySerial.print('\n');
      }
   }
}
 
/**
 * Sets a sensors value
 */
void setSensorValue(String command) {
  int index = command.indexOf(":");
  int id = command.substring(1, index).toInt();
  int value = command.substring(index + 1, command.length()).toInt();
 
  for (int i = 0; i < SENSORSCHANNELS; i++) {
     int channel = i * SENSORSATTRIBUTES;
     int pinNum = sensors[channel].toInt();
     boolean isOutput = sensors[channel + 2].equals(IS_OUTPUT);

     if (isOutput) { 
       if (pinNum == id) {
          String pinType = sensors[channel + 1];
          if (pinType.equals(ANALOG)) {
            analogWrite(pinNum, constrain(value, 0, 255));
          } else if (pinType.equals(DIGITAL)) {
            digitalWrite(pinNum, value > 0 ? HIGH: LOW);
          }
          break;
       }
    }
  }
}
 
/** 
 *  Init the sensor definitions (input/output, default output states, etc)
 */
void initSensors() {
   for (int i = 0; i < SENSORSCHANNELS; i++) {
      int channel = i * SENSORSATTRIBUTES; // Sensor channel index
      int pinNum = sensors[channel].toInt();
      String pinType = sensors[channel + 1];
      boolean isOutput = sensors[channel + 2].equals(IS_OUTPUT);
      int defaultValue = sensors[channel + 3].toInt();
 
      if (isOutput) {
         if (pinType.equals(ANALOG)) {
             pinMode(pinNum, OUTPUT);
             analogWrite(pinNum, constrain(defaultValue, 0, 255));
         } else if (pinType.equals(DIGITAL)) {
             pinMode(pinNum, OUTPUT);
             digitalWrite(pinNum, defaultValue > 0 ? HIGH : LOW);
         }
      }
   }
}
 
void getConfiguration() {
  mySerial.print(CONFIGURATION);
}
