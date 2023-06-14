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

//placeholder for how many places we have free
#define MAX_SPACE_AVAILABLE 10

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

char new_name[8];
char name[] = "ParkFlow";
short int cursor_position = 0;
short int j = 7;
short int index_out_range = 6;
short index_new_name = 0;
bool bar_in = false, bar_out = false;

int curr_available = 5;
int counter = 0;
int period_in = 0,period_out = 0;

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

Serial.print("States: ");
Serial.print(state_in);
Serial.print("   ");
Serial.print(state_out);
Serial.print("  ");

switch(state_in){
  case Idle:
    if(checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1))
      state_in = CarEnter;
  break;

  case CarEnter:
    if(bar_in == false){
      bar_in = raiseBar(servo_in,bar_in);
      break;
    }

    if((checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) && !checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1)) || (period_in == 25 && !(checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1) && checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2)))){
      bar_in = closeBar(servo_in,bar_in);
      period_in = 0;
      state_in = Idle;
    }
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
      period_out = 0;
      state_out = Idle;
    }
    period_out++;

  break;
}

delay(10);
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

  if(getSensorDistance(trigPin1,echoPin1) < 30)
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

//this function raises the bar if called then checks if the car has successfully gone through

bool raiseBar(Servo servo, bool bar){
  for(int i = 0; i< 90; i++){
    delay(10);
    servo.write(i);
    }

  return true;
}

bool closeBar(Servo servo,bool bar){
    for(int i = 90; i>= 0; i--){
      delay(12);
      servo.write(i);
    }

    return false;
}

//SECTION DEDICATED FOR SHIFTING TEXT

void lcdTextInactive()
{
  lcd.setCursor(0,3);
  lcd.print("                    "); // clear last line on display
  
  //text appears from the left
  nameShiftingOnLcd();

  lcd.setCursor(0,1);

  lcd.print("Available:");
  lcd.setCursor(strlen("Available:"),1);
  lcd.print(curr_available);
  
}

void lcdTextActive()
{
  lcd.clear();    
  lcd.setCursor(0,0);
  lcd.print("Status:");
  // ...

  lcd.setCursor(0,1);
  lcd.print("Available:");
  lcd.setCursor(strlen("Available:"),1);
  lcd.print(curr_available);

  nameShiftingOnLcd();
  lcd.clear();
}

void nameShiftingOnLcd()
{
  lcd.setCursor(cursor_position,3);
  if(j > 0)
  {
  for(int i = j; i <= 7; i++)
  {

    new_name[index_new_name] = name[i];
    index_new_name++;
  }
  new_name[index_new_name] = '\0';
  lcd.print(new_name);
  index_new_name = 0;
  j--;
  }
  else
  {
    // Text out of range
    if(cursor_position >= 13)
    {
      for(int i = 0; i <= index_out_range; i++)
      {
        new_name[i] = name[i];
      }
      new_name[index_out_range + 1] = '\0';
      lcd.print(new_name);
      
      index_out_range--;
      cursor_position++;
      if(cursor_position == 21)
      {
        cursor_position = 0;
        j = 7;
        index_out_range = 6;
      }
    }
    else
    {
      lcd.print(name);
      
      cursor_position++;
    }
  }
  delay(200);
}

