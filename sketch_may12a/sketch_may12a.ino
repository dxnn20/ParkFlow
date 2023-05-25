
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
  int dist_test = 0;

  lcdTextInactive();

  servo.write(0);

  //lcd.print(counter);

  //lcd.print(curr_available);

//checking car at the entrance
car = checkCar(TRIGPIN_IN_1,ECHOPIN_IN_1);

//testing purposes

// if car == true => car is at the entrance
// check number and everything

//if statement for testing after confirming the plate number
if(car == true && checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) == false){
  raiseBar();
  time_t start = now();

//check if car entered
  while(checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) == false || checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1) == true){

    lcdTextActive();
    delay(500);
    Serial.print(checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1));
    Serial.print("  ");
    Serial.print(checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2));
    Serial.print("  ");
  
    time_t end = now();
    Serial.print(end - start);
    Serial.print("\n");

    while(checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1) == true && checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) == true)
      delay(1000);

   if((checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1) == false && checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) == true ) || ((end - start) >= 20)){
      delay(100);
      closeBar();
      break;
    }
  }
}

lcdTextInactive();
delay(800);
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
  lcd.print("                    ");// clear first line on display
  //delay 
  
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
  lcd.print("Statut:");
  // ...

  lcd.setCursor(0,1);
  lcd.print("Available:");
  lcd.setCursor(strlen("Available:"),1);
  lcd.print(curr_available);

  nameShiftingOnLcd();
  //delay(20);
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

