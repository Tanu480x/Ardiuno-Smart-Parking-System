#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Initialize LCD (I2C address, columns, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize Servo
Servo myservo;

// Ultrasonic Sensor Pins
const int trigPin1 = 6;  // Trigger pin for Sensor 1 (Entry)
const int echoPin1 = 7;  // Echo pin for Sensor 1 (Entry)
const int trigPin2 = 8;  // Trigger pin for Sensor 2 (Exit)
const int echoPin2 = 9;  // Echo pin for Sensor 2 (Exit)

// Parking Slot Variables
int Slot = 4;   // Total number of parking slots
int flag1 = 0; // Flag for entry sensor
int flag2 = 0; // Flag for exit sensor

// Distance Threshold (in cm)
const int distanceThreshold = 10; // Adjust this value based on your setup

void setup() {
  Serial.begin(9600);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Set ultrasonic sensor pins
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // Attach Servo
  myservo.attach(2); // Servo connected to pin 2
  myservo.write(100); // Initial position (gate closed)

  // Display Welcome Message
  lcd.setCursor(0, 0);
  lcd.print("    ARDUINO    ");
  lcd.setCursor(0, 1);
  lcd.print(" PARKING SYSTEM ");
  delay(2000);
  lcd.clear();
}

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // Convert to cm
}

void smoothServoMove(int startPos, int endPos) {
  int step = (startPos < endPos) ? 1 : -1;
  for (int pos = startPos; pos != endPos; pos += step) {
    myservo.write(pos);
    delay(5); // Adjust delay for smoothness
  }
  myservo.write(endPos); // Ensure it reaches the final position
}

void loop() {
  // Measure distance from Entry Sensor
  long distance1 = getDistance(trigPin1, echoPin1);
  Serial.print("Distance 1: ");
  Serial.println(distance1);

  // Measure distance from Exit Sensor
  long distance2 = getDistance(trigPin2, echoPin2);
  Serial.print("Distance 2: ");
  Serial.println(distance2);

  // Check if a car is detected at the Entry Sensor
  if (distance1 < distanceThreshold && flag1 == 0) {
    Serial.println("Car detected at Entry");
    if (Slot > 0) {
      flag1 = 1;
      smoothServoMove(100, 0); // Open gate smoothly
      Slot = Slot - 1;  // Decrease available slots
      Serial.println("Gate opened for Entry");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("    SORRY :(    ");
      lcd.setCursor(0, 1);
      lcd.print("  Parking Full  ");
      delay(3000);
      lcd.clear();
    }
  }

  // Check if a car is detected at the Exit Sensor
  if (distance2 < distanceThreshold && flag2 == 0) {
    Serial.println("Car detected at Exit");
    flag2 = 1;
    smoothServoMove(100, 0); // Open gate smoothly
    Slot = min(Slot + 1, 4);  // Increase available slots but not exceed 4
    Serial.println("Gate opened for Exit");
  }

  // Reset flags and close gate after car passes
  if (flag1 == 1 || flag2 == 1) {
    delay(3000); // Wait for car to pass
    smoothServoMove(0, 100); // Close gate smoothly
    flag1 = 0;
    flag2 = 0;
    Serial.println("Gate closed");
  }

  // Display available slots on LCD
  lcd.setCursor(0, 0);
  lcd.print("    WELCOME!    ");
  lcd.setCursor(0, 1);
  lcd.print("Slot Left: ");
  lcd.print(Slot);

  delay(1000); // Add a short delay to avoid rapid looping
}
