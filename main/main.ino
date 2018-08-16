#include <Servo.h>

// LED stuff
const int portsAmount = 4;
const int firstPort = 10;
const int frequency = 50;

class Port
{
    public:
    int number;
    bool status;
};


Port ports[portsAmount];
// Direction in which the LEDs are being switched on
// 0= forward; 1= backward
bool direction;

// Moisture sensor stuff
class MoistureSensor {

  private:

    // The analog port where data is sent to by the sensor 
    int port;

    // The boundaries of the values read by the sensors
    // minMoist is the highest value the sensor reads in minimum moisture
    // maxMoist is the lowest value the sensor reads in maximum moisture
    int minMoist = 1010;
    int maxMoist = 260;
    
  public:
    MoistureSensor(int port) {
      this->port = port;
    }

    int getMoisture() {
      int value = analogRead(this->port);
      int percentage = map(value, this->minMoist, this->maxMoist, 0, 100);
      return percentage;
    }
};

class WateringMachine {
  private:
    const int MAX_SENSORS = 6;
    int loadedSensors = 0;
    MoistureSensor* sensors[];
    
  public:
    WateringMachine() {
    }
    
    // Add sensor
    void addSensor(MoistureSensor sensor) {
      if (this->loadedSensors < MAX_SENSORS - 1) {
        this->sensors[loadedSensors] = &sensor;
        this->loadedSensors++;
      }
    }

    // Get sensor percentage, return -999 if sensor is not found
    int readSensorPercentage(int sensorNumber) {
      int percentage = -999;
      if (sensorNumber <= this->loadedSensors) {
        percentage = this->sensors[sensorNumber]->getMoisture();
      }
      return percentage;
    }

};

int sensorsInterfacesTogglePins[6];

void sensorsInterfacesToggle(bool status) {
  for (int i = 0; i < 6; i++) {
    digitalWrite(sensorsInterfacesTogglePins[i], status ? HIGH : LOW);
  }
}
// END of Libraries


WateringMachine wm = WateringMachine();
Servo servo1;
bool servoStatus;

void setup() {
  Serial.begin(9600);
  Serial.println("Setting up...");

  // Init digital pins used as sensor interface toggles
  int firstTogglePin = 2;
  for (int i = 0; i < 6; i++) {
    sensorsInterfacesTogglePins[i] = firstTogglePin;
    pinMode(sensorsInterfacesTogglePins[i], OUTPUT);
    firstTogglePin++;
  }
  
  MoistureSensor s0 = MoistureSensor(0);
  MoistureSensor s1 = MoistureSensor(1);
//  MoistureSensor s2 = MoistureSensor(2);
//  MoistureSensor s3 = MoistureSensor(3);
//  MoistureSensor s4 = MoistureSensor(4);
  wm.addSensor(s0);
  wm.addSensor(s1);

  servo1.attach(9);
  servoStatus = 0;
  
  initLEDs();
}



void loop() {
  Serial.println(millis());

  // Snippet to manage the button activating the servo. 
  // It only serves a purpose while building the hardware, so that the servo can be activated manually.
  int v = digitalRead(4);
  if (v == HIGH) {
    if (servoStatus  == 0) {
      servo1.write(0);
      servoStatus = 1;
    } else {
      servo1.write(90);
      servoStatus = 0;
    }
  }
  // End of the button snippet

  // Switch on sensors
  sensorsInterfacesToggle(1);

  // Delay to give time to the sensor interface to start-up
  delay(500);
  
  int percentage1 = wm.readSensorPercentage(0);
  int percentage2 = wm.readSensorPercentage(1); 
  
  // Delay to give time to the sensor interface to start-up. Is this needed?
  delay(500);

  // Switch off sensors
  sensorsInterfacesToggle(0);
  
  Serial.println(percentage1);
  Serial.println(percentage2);
  
  // LED Code
  runLEDs();
}

void initLEDs() {
  int portNumber = firstPort;
  direction = 0;
  // Init
  for (int i = 0; i < portsAmount; i++) {
      pinMode(portNumber, OUTPUT);
      ports[i].status = 0;
      ports[i].number = portNumber;
      portNumber++;
  }
}

void runLEDs() {
  int currentActivePort = firstPort;
  int counter = 0;
  
  if (!direction) {
    // Starting from the end
    currentActivePort += portsAmount - 1;
  }

  while (counter < portsAmount) {
    // Switch all off
    for (int i = 0; i < portsAmount; i++) {
        digitalWrite(ports[i].number, LOW);
    }

    // Switch the candidate LED on
    digitalWrite(currentActivePort, HIGH);

    // Next one...
    if (direction) {
      currentActivePort++;  
    } else {
      currentActivePort--;  
    }

    // Wait
    delay(frequency);
    counter++;
  }

  // Switch direction
  direction = !direction;
}

