

#include "MotorController.h"
#include <Arduino.h>

MotorController::MotorController() {

  pinMode(build_piston, OUTPUT);
  pinMode(powder_piston, OUTPUT);
  pinMode(recoater_pin, OUTPUT);
  pinMode(direction_pin, OUTPUT);
  pinMode(pulse_pin, OUTPUT);
  pinMode(forwardButtonPin, INPUT_PULLUP);
  pinMode(backwardButtonPin, INPUT_PULLUP);
  pinMode(selectMotorPin, INPUT_PULLUP);
  pinMode(selectSpeedPin, INPUT_PULLUP);
  //All DM542s should be disabled upon boot
  digitalWrite(build_piston, HIGH);
  digitalWrite(powder_piston, HIGH);
  digitalWrite(recoater_pin, HIGH);

}

void MotorController::move(int select, int steps, int freq, int direction) {
  if (select < 2 || select >= 5) {
    return; // Invalid motor selection
  }
   enableMotor(select);

   digitalWrite(direction_pin, direction);
  
  for(int i = 0; i < steps; i++) {
    pulse(freq);
    if (i > steps - 80) delay(2);
    //delayMicroseconds(1000000 / frequency);
  }
  
   disableAllMotors();
}

void MotorController::pulse(int frequency = 500) {
  digitalWrite(pulse_pin, HIGH);
  //GPIO6_DR |= (1 << pulse_pin);
  delayMicroseconds(1000000 / (frequency*2));
  digitalWrite(pulse_pin, LOW);
  //GPIO6_DR &= ~(1 << pulse_pin);
  delayMicroseconds(1000000 / (frequency*2));
}

void MotorController::manualOverride() {
  static unsigned long motorDebounce = 0;
  static unsigned long speedDebounce = 0;
  static int lastSelectMotor = HIGH;
  static int lastSelectSpeed = HIGH;
  int selectMotor = digitalRead(selectMotorPin);
  //int selectMotor = GPIO6_DR & (1 << selectMotorPin);
  int selectSpeed = digitalRead(selectSpeedPin);
  //int selectSpeed = GPIO6_DR & (1 << selectSpeedPin);
  
  if (selectMotor != lastSelectMotor) {
    motorDebounce = millis();
  }

  if (selectSpeed != lastSelectSpeed) {
    speedDebounce = millis();
  }
  
  if ((millis() - motorDebounce) > 100) {
    if (selectMotor == LOW) {
      toggleMotor();
      delay(500);
    }
  }

  if ((millis() - speedDebounce) > 100) {
    if (selectSpeed == LOW) {
      toggleSpeed();
      delay(500);
    }
  }
  
  lastSelectMotor = selectMotor;
  lastSelectSpeed = selectSpeed;

  //if (selectedMotor != -1) {
    int forwardState = digitalRead(forwardButtonPin);
    //int forwardState = GPIO6_DR & (1 << forwardButtonPin);
    int backwardState = digitalRead(backwardButtonPin);
    //int backwardState = GPIO6_DR & (1 << backwardButtonPin);
    
    if (forwardState == LOW) {
      digitalWrite(direction_pin, HIGH);
      //GPIO6_DR |= (1 << direction_pin);
      pulse(motorSpeeds[speedSetting]);
    } else if (backwardState == LOW) {
      digitalWrite(direction_pin, LOW);
      //GPIO6_DR &= ~(1 << direction_pin);
      pulse(motorSpeeds[speedSetting]);
    }
 // }
}

void MotorController::toggleMotor() {
  selectedMotor++;
  if (selectedMotor >= 5) {
    selectedMotor = 1; // None state
  }

  for (int i = 2; i < 5; i++) {
    digitalWrite(i, (i == selectedMotor) ? LOW: HIGH);
  }
}

void MotorController::toggleSpeed(){
  speedSetting = ++speedSetting % ARRAY_LENGTH(motorSpeeds);
}

void MotorController::enableMotor(int select) {
  for (int i = 2; i < 5; i++) {
    digitalWrite(i, (i == select) ? LOW : HIGH);
  }
}

void MotorController::disableAllMotors() {
  for (int i = 2; i < 5; i++) {
    digitalWrite(i, HIGH);
  }
}

