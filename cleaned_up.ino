#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <Servo.h>

// Sensor pins
#define TRIGPIN_OUT_1 3
#define ECHOPIN_OUT_1 4
#define TRIGPIN_OUT_2 5 
#define ECHOPIN_OUT_2 6 
#define TRIGPIN_IN_1 9
#define ECHOPIN_IN_1 10
#define TRIGPIN_IN_2 11
#define ECHOPIN_IN_2 12

// Servo pins
#define SERVO_IN 8
#define SERVO_OUT 7

// Max space & LED
#define MAX_LOTS 20
#define LED 2

enum ParkingState {
  Idle,
  Active,
};

// Global Variables
ParkingState state_in = Idle;
ParkingState state_out = Idle;
LiquidCrystal_I2C lcd(0x27, 20, 4);

Servo servo_in;
Servo servo_out;

int curr_available = MAX_LOTS;
unsigned long entryTimeout = 0;
unsigned long exitTimeout = 0;

byte occupied[] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };
byte empty[] = { B11111, B10001, B10001, B10001, B10001, B10001, B10001, B11111 };

void setup() {
  Serial.begin(115200);

  // Setup Servos
  servo_in.attach(SERVO_IN);
  servo_in.write(0);
  servo_out.attach(SERVO_OUT);
  servo_out.write(0);

  // Setup LCD
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.createChar(0, occupied);
  lcd.createChar(1, empty);
  printParking(MAX_LOTS - curr_available);

  // Setup Pins
  pinMode(TRIGPIN_IN_1, OUTPUT);
  pinMode(ECHOPIN_IN_1, INPUT);
  pinMode(TRIGPIN_IN_2, OUTPUT);
  pinMode(ECHOPIN_IN_2, INPUT);
  
  pinMode(TRIGPIN_OUT_1, OUTPUT);
  pinMode(ECHOPIN_OUT_1, INPUT);
  pinMode(TRIGPIN_OUT_2, OUTPUT);
  pinMode(ECHOPIN_OUT_2, INPUT);
  
  pinMode(LED, OUTPUT);
}

void loop() {
  handleEntry();
  handleExit();
}

// --- CORE LOGIC ---

void handleEntry() {
  switch (state_in) {
    case Idle:
      // If there is space and a car arrives at the entry gate
      if (curr_available > 0 && checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1)) {
        state_in = Active;
        
        // Standalone simulation of processing/validation
        printProcessing();
        delay(800); 
        textValid();
        
        digitalWrite(LED, HIGH);
        raiseBar(servo_in);
        entryTimeout = millis(); // Start timeout timer
      }
      // If the lot is full and a car arrives (Optional feedback)
      else if (curr_available == 0 && checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1)) {
         textInvalid(); // Shows Invalid/Full
         delay(2000);
         printParking(MAX_LOTS - curr_available);
      }
      break;

    case Active:
      // If car passes through to the inside (Hits IN_2, clears IN_1)
      if (checkCar(TRIGPIN_IN_2, ECHOPIN_IN_2) && !checkCar(TRIGPIN_IN_1, ECHOPIN_IN_1)) {
        closeBar(servo_in);
        digitalWrite(LED, LOW);
        curr_available--;
        state_in = Idle;
        printParking(MAX_LOTS - curr_available);
      }
      // If car backs away or takes too long (5 second timeout)
      else if (millis() - entryTimeout > 5000) {
        closeBar(servo_in);
        digitalWrite(LED, LOW);
        state_in = Idle;
        printParking(MAX_LOTS - curr_available);
      }
      break;
  }
}

void handleExit() {
  switch (state_out) {
    case Idle:
      // Car approaches the exit gate
      if (checkCar(TRIGPIN_OUT_1, ECHOPIN_OUT_1)) {
        state_out = Active;
        raiseBar(servo_out);
        exitTimeout = millis(); // Start timeout timer
      }
      break;

    case Active:
      // Car leaves completely (Hits OUT_2, clears OUT_1)
      if (checkCar(TRIGPIN_OUT_2, ECHOPIN_OUT_2) && !checkCar(TRIGPIN_OUT_1, ECHOPIN_OUT_1)) {
        closeBar(servo_out);
        if (curr_available < MAX_LOTS) {
          curr_available++;
        }
        state_out = Idle;
        printParking(MAX_LOTS - curr_available);
      }
      // If car backs away from exit (5 second timeout)
      else if (millis() - exitTimeout > 5000) {
        closeBar(servo_out);
        state_out = Idle;
      }
      break;
  }
}

// --- HELPER FUNCTIONS ---

// Returns distance in cm. Added timeout to pulseIn to prevent code freezing.
unsigned short getSensorDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);
  
  // 30000us timeout prevents hanging if no echo is received
  long duration = pulseIn(echoPin, HIGH, 30000); 
  if (duration == 0) return 999; // Return out-of-range if no ping back
  
  return ((duration / 2) / 29.1);
}

// Executes 5 scans. If 3 consecutive ones are < 30cm, car is confirmed.
bool checkCar(int trigPin1, int echoPin1) {
  int counter = 0;  
  for (int i = 0; i < 5; i++) {
    if (getSensorDistance(trigPin1, echoPin1) < 30) {
      counter++;
    } else {
      counter = 0;
    }
    delay(10);
  }
  return (counter >= 3);
}

void raiseBar(Servo &servo) {
  for (int i = 0; i < 90; i += 2) {
    delay(8);
    servo.write(i);
  }
}

void closeBar(Servo &servo) {
  for (int i = 90; i >= 0; i -= 2) {
    delay(8);
    servo.write(i);
  }
}

// --- DISPLAY FUNCTIONS ---

void textValid() {
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("Welcome!");
}

void textInvalid() {
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("Invalid");
}

void printProcessing() {
  char str[] = "Processing...";
  int n = strlen(str);
  lcd.clear();
  lcd.setCursor(10 - (n / 2), 1);
  for (int i = 0; i < n; i++) {
    lcd.print(str[i]);
    delay(20); // Sped up slightly for Tinkercad
  }
}

void printParking(int occupiedSpaces) {
  int printed = 0;

  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("ParkFlow");

  lcd.setCursor(7 - strlen("Available:") / 2, 3);
  lcd.print("Available: ");
  lcd.print(curr_available);

  // Draw the graphical blocks
  for (int i = 0; i < 4; i++) {
    for (int j = 15; j < 20; j++) {
      lcd.setCursor(j, i);
      if (printed < occupiedSpaces) {
        lcd.write(0);
        printed++;
      } else {
        lcd.write(1);
      }
    }
  }

  // Show full if 0 spaces
  if (curr_available == 0) {
    lcd.setCursor(16, 2);
    lcd.print("FULL");
  }
}
