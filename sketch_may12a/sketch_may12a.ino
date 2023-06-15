#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <Servo.h>
#include <TimeLib.h>

// sensor pins
#define TRIGPIN_OUT_1 3
#define ECHOPIN_OUT_1 4
#define TRIGPIN_OUT_2 5 
#define ECHOPIN_OUT_2 6 
#define TRIGPIN_IN_1 9
#define ECHOPIN_IN_1 10
#define TRIGPIN_IN_2 11
#define ECHOPIN_IN_2 12

//servo pin
#define SERVO_IN 8
#define SERVO_OUT 7

enum ParkingState {
  Idle,
  CarEnter,
  CarExit
};

// Variables

ParkingState state_in = 0, state_out = 0;
LiquidCrystal_I2C lcd(0x27,20,4);
Servo servo_in;
Servo servo_out;

byte occupied[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte empty[] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111
};

int period_in = 0,period_out = 0, curr_available = 1;
bool bar_in = false, bar_out = false;

void setup() {
  Serial.begin(9600);
  servo_in.attach(SERVO_IN);
  servo_in.write(0);
  servo_out.attach(SERVO_OUT);
  servo_out.write(0);

  bar_in = false;
  bar_out = false;

  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.setCursor(7 - strlen("Available:")/2, 3);
  lcd.print("Available:");
  lcd.createChar(0, occupied);
  lcd.createChar(1, empty);

  pinMode(TRIGPIN_IN_1, OUTPUT);
  pinMode(ECHOPIN_IN_1, INPUT);

  pinMode(TRIGPIN_IN_2, OUTPUT);
  pinMode(ECHOPIN_IN_2, INPUT);

  pinMode(TRIGPIN_OUT_1, OUTPUT);
  pinMode(ECHOPIN_OUT_1, INPUT);

  pinMode(TRIGPIN_OUT_2, OUTPUT);
  pinMode(ECHOPIN_OUT_2, INPUT);

  lcd.write(0);

}

void loop() {

Serial.print("States: ");
Serial.print(state_in);
Serial.print("   ");
Serial.print(state_out);
Serial.print("  ");

switch(state_in){
  case Idle:
  printParking(20 - curr_available);

  if ( curr_available == 0)
    break;
    
    if(checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1)){
      state_in = CarEnter;
      lcd.clear();
      Serial.write('s');
    }
  break;

  case CarEnter:
    if(bar_in == false  && Serial.read() == 'v'){
      bar_in = raiseBar(servo_in,bar_in);
      textValid();
      break;
    }
    else {
      textInvalid();
      state_in = Idle;
      break;
    }

    if( bar_in == true &&  ((checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) && !checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1)))){
      bar_in = closeBar(servo_in,bar_in);
      curr_available--;
      period_in = 0;
      state_in = Idle;
      lcd.clear();
      lcd.setCursor(6,1);
      lcd.print("ParkFlow");

    }
    else if(bar_in == true && ( period_in == 25 && !(checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1) && checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2)))){
      bar_in = closeBar(servo_in,bar_in);
      period_in = 0;
      state_in = Idle;
      lcd.clear();
      lcd.setCursor(6,1);
      lcd.print("ParkFlow");
    }

    if( bar_in == false && period_in == 25)
      state_in = Idle;

    period_in++ ;
  break;

}

Serial.print("Sensor exit: ");
Serial.print(checkCar(TRIGPIN_OUT_1, ECHOPIN_OUT_1));
Serial.print("   ");
Serial.print(checkCar(TRIGPIN_OUT_2, ECHOPIN_OUT_2));
Serial.print("\n");

switch(state_out){

  case Idle:
    if(checkCar(TRIGPIN_OUT_1, ECHOPIN_OUT_1))
      state_out = CarExit;
  break;
  
  case CarExit: 
  if(!bar_out){
      bar_out = raiseBar(servo_out,bar_out);
      break;
    }
    if((checkCar(TRIGPIN_OUT_2, ECHOPIN_OUT_2) && !checkCar(TRIGPIN_OUT_1, ECHOPIN_OUT_1)) || (period_out == 25 && !(checkCar(TRIGPIN_OUT_1, ECHOPIN_OUT_1) && checkCar(TRIGPIN_OUT_2, ECHOPIN_OUT_2)))){
      bar_out = closeBar(servo_out,bar_out);
      curr_available++;
      period_out = 0;
      state_out = Idle;
    }
    period_out++;

  break;
  }

}

//function that returns the distance between a sensor and the closes object to it
unsigned short getSensorDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);

  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);

  return ((duration / 2) / 29.1);

}

//this function executes 5 scans and if 3 consecutive ones return a positive result then a car is stationary at the entrance / exit
bool checkCar(int trigPin1, int echoPin1){
  int counter = 0;  

for(int i = 0 ; i < 5; i++){

  if(getSensorDistance(trigPin1,echoPin1) < 30)
    counter++;
  else 
   counter = 0;

  delay(2);
}

if( counter >= 3)
  return true;
else 
  return false;
}

bool raiseBar(Servo servo, bool bar){
  for(int i = 0; i< 90; i += 2){
    delay(8);
    servo.write(i);
    }

  return true;
}

bool closeBar(Servo servo,bool bar){
  for(int i = 90; i>= 0; i -= 2){
    delay(8);
    servo.write(i);
  }

    return false;
}

bool textValid(){
  lcd.clear();
  lcd.setCursor(6,1);
  lcd.print("Welcome!");

      return true;
}

bool textInvalid(){
  lcd.clear();
  lcd.setCursor(6,1);
  lcd.print("Invalid");

  return false;
}

void printParking(int n ){
  int printed = 0;

  lcd.clear();
  lcd.setCursor(2,1);
  lcd.print("ParkFlow");

  lcd.setCursor(7 - strlen("Available:")/2, 3);
  lcd.print("Available:");
  lcd.print(curr_available);

  for(int i = 0 ; i < 4 ;i++ )
    for(int j = 15 ; j < 20; j++ ){
      lcd.setCursor(j,i);
      if( printed < n){
        lcd.write(0);
        printed++;
      }
      else 
      lcd.write(1);
    }

    if( n == 20){
      lcd.setCursor(9,0);
      lcd.print("FULL->");
      }
}