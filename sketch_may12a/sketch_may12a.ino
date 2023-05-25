
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
#define SERVO 8

//placeholder for how many places we have free
#define MAX_SPACE_AVAILABLE 10

// Variables
LiquidCrystal_I2C lcd(0x27,20,4);
Servo servo;

char new_name[8];
char name[] = "ParkFlow";
short int cursor_position = 0;
short int j = 7;
short int index_out_range = 6;
short index_new_name = 0;

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
  //starting values coresponding to the idle ones
  lcdTextInactive();
  servo.write(0);

//checking car at the entrance
// if car == true => car is at the entrance, else , no car
car = checkCar(TRIGPIN_IN_1,ECHOPIN_IN_1);

//testing purposes
// if car == true => car is at the entrance
// check number and everything

//this if statement is temporary to test the function
if(car == true && checkCar(TRIGPIN_IN_2,  ECHOPIN_IN_2) == false)
  carEnter(car, TRIGPIN_IN_1, ECHOPIN_IN_1, TRIGPIN_IN_2, ECHOPIN_IN_2);

//checking if any car wants to leave
car = checkCar(TRIGPIN_OUT_1,ECHOPIN_OUT_1);

//no need for verification or anything so car can freely leave, the illusion of free will, they MUST leave
if(car == true && checkCar(TRIGPIN_OUT_2, ECHOPIN_IN_2) == false)
  carEnter(car,TRIGPIN_IN_1,ECHOPIN_IN_1,TRIGPIN_IN_2,ECHOPIN_IN_2);

lcdTextInactive();
delay(300);
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
void carEnter(bool car, int trig_pin1 ,int echo_pin1, int trig_pin2,int echo_pin2){

  //if statement for testing after confirming the plate number
  raiseBar();
  time_t start = now();

//check if car entered
  while(checkCar(trig_pin2,  echo_pin2) == false || checkCar(trig_pin1, echo_pin1) == true){

    //testing purposes
    lcdTextActive();
    delay(500);
    Serial.print(checkCar(trig_pin1, echo_pin1));
    Serial.print("  ");
    Serial.print(checkCar(trig_pin2,  echo_pin2));
    Serial.print("  ");
  
    time_t end = now();
    Serial.print(end - start);
    Serial.print("\n");

    while(checkCar(trig_pin1, echo_pin1) == true && checkCar(trig_pin2,  echo_pin2) == true)
      delay(1000);

   if((checkCar(trig_pin1, echo_pin1) == false && checkCar(trig_pin2,  echo_pin2) == true ) || ((end - start) >= 20)){
      delay(100);
      closeBar();
      break;
    }
  }
}

void raiseBar(){
  for(int i = 0; i< 90; i++){
    delay(10);
    servo.write(i);
    }
}

void closeBar(){
    for(int i = 90; i>= 0; i--){
      delay(12);
      servo.write(i);
    }
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

