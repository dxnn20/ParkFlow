
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <Servo.h>

#define TRIGPIN_OUT_1 3
#define ECHOPIN_OUT_1 4
#define TRIGPIN_OUT_2 5 
#define ECHOPIN_OUT_2 6 
#define TRIGPIN_IN_1 9
#define ECHOPIN_IN_1 10
#define TRIGPIN_IN_2 11
#define ECHOPIN_IN_2 12

#define SERVO 8
#define MAX_SPACE_AVAILABLE 10


LiquidCrystal_I2C lcd(0x27,20,4);
Servo servo;
int curr_available = 5;
int counter = 0;
int period = 1000;
bool car = false;
unsigned long curr_time;


void setup() {
  Serial.begin(9600);
  servo.attach(SERVO);
  servo.write(0);

  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.print("Available:");

  pinMode(TRIGPIN_IN_1, OUTPUT);
  pinMode(ECHOPIN_IN_1, INPUT);

  pinMode(TRIGPIN_IN_2, OUTPUT);
  pinMode(ECHOPIN_IN_2, INPUT);

  pinMode(TRIGPIN_OUT_1, OUTPUT);
  pinMode(ECHOPIN_OUT_1, INPUT);

  pinMode(TRIGPIN_OUT_2, OUTPUT);
  pinMode(ECHOPIN_OUT_2, INPUT);

}

void loop() {
  int dist_test = 0;

  servo.write(0);
  lcd.setCursor(0,1);

  lcd.setCursor(strlen("Available:"),0);
  lcd.print("  ");
  lcd.setCursor(strlen("Available:"),0);

  //lcd.print(counter);

  //lcd.print(curr_available);

//checking car at the entrance
car = checkCar(TRIGPIN_IN_1,ECHOPIN_IN_1);

//testing purposes
if(car == true)
  lcd.print(1);
else 
  lcd.print(0);

// if car == true => car is at the entrance
// check number and everything

//if statement for testing after confirming the plate number
if(car == true)
  raiseBar();

//check if car entered
while(checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) == false ){
  delay(1000);
  
  if(checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1) == false)
    break;
}

closeBar();

delay(1000);
}

unsigned short getSensorDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);

  return ((duration / 2) / 29.1);

}

bool checkCar(int trigPin1, int echoPin1){
  counter = 0;

for(int i = 0 ; i < 5; i++){
  delay(10);

  if(getSensorDistance(trigPin1,echoPin1) < 20)
    counter++;
  else 
   counter = 0;

  delay(10);
}

if( counter >= 3)
  return true;
else 
  return false;
}

void raiseBar(){
  for(int i = 0; i< 90; i++){
    delay(10);
    servo.write(i);
    }
}
void closeBar(){
    for(int i = 90; i>= 0; i--){
      delay(10);
      servo.write(i);
    }
}
//bar logic:
// - a few scans before it runs the camera check
// -  if distance <= 15 run scan

//replace delay: 
