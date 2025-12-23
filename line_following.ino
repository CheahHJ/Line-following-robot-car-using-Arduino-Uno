#include <LiquidCrystal.h>
#include <Wire.h>
#include <PinChangeInterrupt.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

bool hasStoppedFor20cm = false;

unsigned long startTime = 0;
unsigned long currentTime = 0;
unsigned long lastLCDUpdate = 0; 
bool isStopped = false;

const int LEFT_IR = A2;
const int RIGHT_IR = A3;

const uint8_t encoderRightPin = A5;
const uint8_t encoderLeftPin = A4;

volatile long rightCount = 0;
volatile long leftCount = 0;

const float pulsesPerRev = 40.0; 
const float wheelDiameter = 6.0;
const float wheelCircumference = 3.1416 * wheelDiameter; 

const int ENA = 3;   
const int IN1 = 1;   
const int IN2 = 2;  

const int ENB = 11;  
const int IN3 = 12;  
const int IN4 = 13;  

int baseSpeed = 85; 
int turnSpeed = 180; 

void onRightPulse() {
  rightCount++;
}

void onLeftPulse() {
  leftCount++;
}

void setup() {
  lcd.begin(16, 2);
  lcd.print("Ready...");
  
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  pinMode(encoderLeftPin, INPUT_PULLUP); 
  pinMode(encoderRightPin, INPUT_PULLUP);

  attachPCINT(digitalPinToPCINT(encoderLeftPin), onLeftPulse, CHANGE);
  attachPCINT(digitalPinToPCINT(encoderRightPin), onRightPulse, CHANGE);

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  delay(1000);
  lcd.clear();
  startTime = millis();
}

void moveForward(int speedLeft, int speedRight) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(ENA, speedLeft);

  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENB, speedRight);
}

void turnRight() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  analogWrite(ENA, 195);

  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
  analogWrite(ENB, 235); 
  delay (190);
}

void turnLeft() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(ENA, 220);

  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENB, 180);

  delay(170);
}

void stopCar() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  
  isStopped = true;
}

void loop() {
  if (isStopped) {
    stopCar();
    
    if (millis() % 1000 == 0) {
        lcd.setCursor(11, 0);
        lcd.print("DONE");
    }
    return;
  }

  float rightDistance = ((float)rightCount / pulsesPerRev) * wheelCircumference;
  float leftDistance  = ((float)leftCount  / pulsesPerRev) * wheelCircumference;
  float avgDistance   = (rightDistance + leftDistance) / 2.0;

  currentTime = millis() - startTime;

  if (millis() - lastLCDUpdate > 200) {
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(currentTime / 1000);
    lcd.print(".");
    lcd.print((currentTime % 1000) / 100);

    lcd.setCursor(0, 1);
    lcd.print("D:");
    lcd.print(avgDistance, 1);
    lcd.print("cm");
    
    lastLCDUpdate = millis();
  }

  int leftValue = analogRead(LEFT_IR);
  int rightValue = analogRead(RIGHT_IR);
  int Threshold = 300;

  bool leftOnBlack = (leftValue < Threshold);
  bool rightOnBlack = (rightValue < Threshold);

  if (leftOnBlack && rightOnBlack) {
    stopCar(); 
  }

  else if (!leftOnBlack && !rightOnBlack) {
    moveForward(baseSpeed, baseSpeed);
  }

  else if (leftOnBlack && !rightOnBlack) {
    turnLeft();
  }

  else if (!leftOnBlack && rightOnBlack) {
    turnRight();
  }
}