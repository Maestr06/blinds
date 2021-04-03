#include <Stepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <BMx280TwoWire.h>
#define I2C_ADDRESS 0x76

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
// for your motor
// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

int stepCount = 0;  // number of steps the motor has taken

BMx280TwoWire bmx280(&Wire, I2C_ADDRESS);

void setup() {
  Serial.begin(9600);
  myStepper.setSpeed(14);
  pinMode(7, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  while (!Serial);
  Wire.begin();
  //begin() checks the Interface, reads the sensor ID (to differentiate between BMP280 and BME280)
  //and reads compensation parameters.
  if (!bmx280.begin())
  {
    Serial.println("begin() failed. check your BMx280 Interface and I2C Address.");
    while (1);
  }
  //reset sensor to default parameters.
  bmx280.resetToDefaults();
  bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
  bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);
  if (bmx280.isBME280()){
    bmx280.writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
  }
}

void loop() {
  if (!bmx280.measure())
  {
    Serial.println("could not start measurement, is a measurement already running?");
    return;
  }
  do
  {
    delay(100);
  } while (!bmx280.hasValue());
  int sz = digitalRead(6);
  int si = digitalRead(7);
  Serial.println(digitalRead(6));
  
  
  Serial.print("Temperature: "); Serial.println(bmx280.getTemperature());
  if (bmx280.getTemperature() > 25) {
    myStepper.step(-2048*5);
    }
  if (si == LOW) {
    myStepper.step(-512);
  }
  // delay(10000);
  if (sz == LOW) {
    myStepper.step(512);
    Serial.println("dupa");
  }
}
