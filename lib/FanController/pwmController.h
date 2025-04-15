#pragma once

#ifndef FAN_h
#define FAN_h

#include <Arduino.h>

// 1200us ~ Idle speed
#define DEFAULT_PW 1200
// 1.2ms at 50Hz (246/4095 x 20ms)
#define FAN_IDLE 246
// 1.0ms at 50Hz (206/4095 x 20ms)
#define FAN_STOP 206
// 2.0ms at 50Hz (408/4095 x 20ms)
#define TIM_MAX 4095
// Max value (PWM output inverted)
#define MAX_12BIT 4095
// 4095 base value
#define PWM_BIT_RES 12
// Signal frequency ~ 50Hz
#define ESC_FREQ 50.5
// PWM pins for respective components
#define FAN_SEL 7
#define CAM_LASER_SEL 8


 class PWMController {


  public:
    PWMController(uint8_t pin);
    void initPWM(uint16_t pulseWidth);
    void disablePWM();
    void setWidth(uint16_t pulseWidth);

  private:

    uint8_t pwmPin;

};

#endif
