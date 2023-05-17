
// #include <Servo.h>

// int pos = 0;

// Servo servo;

// void setup() {
//   
// }

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <Servo.h>

#define TRIGPIN_OUT_1 3
#define ECHOPIN_OUT_1 4
#define TRIGPIN_OUT_2 5 // PROBLEMATIC
#define ECHOPIN_OUT_2 6 //  PB
#define TRIGPIN_IN_1 9
#define ECHOPIN_IN_1 10
#define TRIGPIN_IN_2 11
#define ECHOPIN_IN_2 12
#define SERVO 8
#define MAX_SPACE_AVAILABLE 10

LiquidCrystal_I2C lcd(0x27,20,4);
Servo servo;
int curr_available = 5;
int counter_test = 0;

void setup() {
  Serial.begin(9600);
  servo.attach(SERVO);
  servo.write(0);

  lcd.init();
  lcd.clear();         
  lcd.backlight();

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


  lcd.setCursor(0,0);

  lcd.print("Available:");
  lcd.setCursor(strlen("Available:"),0);

  lcd.print(curr_available);
  lcd.setCursor(0,1);

  delay(10);
  lcd.setCursor(0,1);
  lcd.print("Sensor ");
  lcd.print(TRIGPIN_OUT_2);
  lcd.print(",");
  lcd.print( ECHOPIN_OUT_2 );
  lcd.print(":");
  
  dist_test = getSensorDistance(TRIGPIN_OUT_2, ECHOPIN_OUT_2);
  lcd.print(dist_test);
  lcd.print("  ");
  

  if(dist_test <=15 )
    counter_test++;
  else 
    counter_test = 0;

lcd.print(counter_test);

  if(counter_test == 3){

    for(int i = 0; i< 90; i++){
      delay(10);
      servo.write(i);
    }
    counter_test = 0;
    

    while(dist_test <=15){
      dist_test = getSensorDistance(TRIGPIN_OUT_2, ECHOPIN_OUT_2);
      delay(350);

    }
    for(int i = 90; i>= 0; i--){
      delay(10);
      servo.write(i);
    }
  }
  
  Serial.print(dist_test);
  delay(1000);
  
  lcd.clear();

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

//bar logic:
// - a few scans before it runs the camera check
// -  if distance <= 15 run scan
