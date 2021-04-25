#include <Stepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <BMx280TwoWire.h>
#include <BH1750.h>
#define I2C_ADDRESS 0x76



int stepCount = 0;  // number of steps the motor has taken
const int dirpin = 4;
const int steppin = 3;

BMx280TwoWire bmx280(&Wire, I2C_ADDRESS);
void light_read(); //checking current lux value
void temp_read(); //checking current temp
void left()
void right()


void setup() {
  Serial.begin(9600);
   pinMode(dirpin, OUTPUT);
  pinMode(steppin, OUTPUT);
  pinMode(5, INPUT_PULLUP);
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
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);

  Serial.println(F("BH1750 One-Time Test"));
}

void loop() {   
}

void light_read() {
  lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
  while (!lightMeter.measurementReady(true)) {
    yield();
  }
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
}

void temp_read{
if (!bmx280.measure())
  {
    Serial.println("could not start measurement, is a measurement already running?");
    return;
  }
  do
  {
    delay(100);
  } while (!bmx280.hasValue());

  Serial.print("Temperature: "); Serial.println(bmx280.getTemperature());
}  

void right() {
  digitalWrite(dirpin, LOW);
  while(digitalRead(5) == LOW) {
    digitalWrite(steppin,HIGH); 
    delayMicroseconds(1000); 
    digitalWrite(steppin,LOW); 
    delayMicroseconds(1000);
  }
}

void left() {
  digitalWrite(dirpin, HIGH);
  while(digitalRead(6) == LOW) {
    digitalWrite(steppin,HIGH); 
    delayMicroseconds(1000); 
    digitalWrite(steppin,LOW); 
    delayMicroseconds(1000);
  }
