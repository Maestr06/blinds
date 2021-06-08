#include <SoftwareSerial.h>
#include <Arduino.h>
#include <Wire.h>
#include <BMx280TwoWire.h>
#include <BH1750.h>
#define I2C_ADDRESS 0x76

bool closed = true;
bool temp_closed = false;
bool autonomy = true;
const char left = 'l';
const char right = 'r';
int stepCount = 0;  // number of steps the motor has taken
const int dirpin = 4; //HIGH is left rotation, LOW is right rotation
const int steppin = 3;
const int autopin = 11;
const int hallpin = 12;
float temperature = 20;
float lux;
const int MS1 = 8;
const int MS2 = 9;
const int MS3 = 10;
const int speedofmotor = 60;
unsigned long long int rotation = 6400; // steps for one rotation on 1/32 microstep mode
unsigned int motor_pos = 0;
unsigned int steps = 0;
unsigned long old_time = 0;
unsigned long pos_time = 0;
unsigned long read_time = 0;
char incoming_value = 0;
bool bt = false;

BMx280TwoWire bmx280(&Wire, I2C_ADDRESS);
BH1750 lightMeter;
void light_read(); // checking current lux value
void temp_read(); // checking current temp
void turn_left(); // function to continously turn left
void turn_right(); // function to continously turn right
void rotate(char rotate_dir, float rotations); // function that takes care of rotating by given amount of rotations
void go_home(); // function that rotates right until it hits Hall sensor

SoftwareSerial mySerial(13, 2); // RX, TX

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  pinMode(dirpin, OUTPUT);
  pinMode(steppin, OUTPUT);
  pinMode(autopin, INPUT_PULLUP);
  pinMode(hallpin, INPUT);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);
  
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
  go_home();
  motor_pos = 0;
}

void loop() {
  if(mySerial.available() > 0)  
  {
    incoming_value = mySerial.read();
    Serial.println(incoming_value);
    if (incoming_value == '1') {
      bt = true;
      turn_right();
      bt = false;        
    }
    else if (incoming_value == '0') {
      bt = true;
      turn_left();
      bt = false;
    }
    else if (incoming_value == '2') {
      if (autonomy == true)
        autonomy = false;
      else
        autonomy = true;
    }
    else if (incoming_value == '3') {
      Serial.println(mySerial.print(int(lightMeter.readLightLevel())));
      mySerial.flush();
    }
    else if (incoming_value == '4') {
      mySerial.print(long(bmx280.getTemperature()));
      mySerial.flush();
    }
  }
  
//  Serial.println(millis() - old_time);
  if (millis() - read_time > 10000) {
    temp_read();
    light_read();
    read_time = millis();
  }
//  Serial.print(digitalRead(hallpin));Serial.println(analogRead(A0));
  turn_right();
  turn_left();
  switch_mode();
  pos_check();
  if (millis() - old_time > 10000) {
    if (autonomy == true) {
      if (lux >= 100 && closed == true && temp_closed == false) {
        rotate('l', 7);
        closed = false;
        motor_pos = 7;
      }
      else if (lux < 100 && closed == false && temp_closed == false) {
        rotate('r', 7);
        closed = true;
        motor_pos = 0;
      }
      else if (lux < 100 && closed == false && temp_closed == true) {
        rotate('r', 10);
        closed = false;
        motor_pos = 0;
      }
      else if (temperature >= 26 && closed == false && temp_closed == false) {
        rotate('l', 3);
        temp_closed = true;
        motor_pos = 10;
      }
      else if (temperature < 26 && closed == false && temp_closed == true) {
        rotate('r', 3);
        temp_closed = false;
        motor_pos = 7;
      }
    }
  old_time = millis();
  }
  if (millis() - pos_time > 1000) {
    Serial.print("Pozycja: ");Serial.println(motor_pos);
    pos_time = millis();
  }
}

void light_read() {
  lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
  while (!lightMeter.measurementReady(true)) {
    yield();
  }
  lux = lightMeter.readLightLevel();
  Serial.print("Light: "); Serial.print(lux); Serial.println(" lx");
}

void temp_read(){
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
  temperature = bmx280.getTemperature();
}

void rotate(char rotate_dir, float rotations) {
  if (rotate_dir == left) {
    digitalWrite(dirpin, HIGH);
  }
  else if (rotate_dir == right) {
    digitalWrite(dirpin, LOW);
  }
  Serial.println(int(rotation*rotations));
  for (long i = 0; i < (rotation*rotations); i++) {
    if (i % 1000 == 0) {
      Serial.println(i);
    }
    digitalWrite(steppin,HIGH); 
    delayMicroseconds(speedofmotor); 
    digitalWrite(steppin,LOW); 
    delayMicroseconds(speedofmotor);
  }
}

void turn_right() {
  if (digitalRead(5) == LOW || bt == true) {
    if (motor_pos == 0)  {
      Serial.println("Granica zakresu!!!");
    }
     else {
      rotate('r', 1);
      motor_pos--;
      autonomy = false;
    }
  }
}

void turn_left() {
  if (digitalRead(6) == LOW || bt == true) {
    if (motor_pos == 13){
      Serial.println("Granica zakresu!!!");
    }
     else {
      rotate('l', 1);
      motor_pos++;
      autonomy = false;
    }
  }
}

void pos_check() {
  if (motor_pos == 7){
    closed = false;
  }
  else if (motor_pos == 10){
    temp_closed = true;
  }
  else{
    temp_closed = false;
  }
}

void switch_mode() {
  if(digitalRead(autopin) == LOW) {
    if (autonomy == true){
      autonomy = false;
      mySerial.print(long(bmx280.getTemperature()));
      mySerial.flush();
    }
    else if (autonomy == false){
      autonomy = true;
      mySerial.print(long(bmx280.getTemperature()));
      mySerial.flush();
      go_home();
    }
  }
  delay(10);
}

void go_home() {
  while(digitalRead(hallpin) == LOW) {
    Serial.print(digitalRead(hallpin));Serial.println(analogRead(A0));
    rotate('r', 1.0/4.0);
  }
  closed = true;
  motor_pos = 0;
}
