#include "pwmController.h"

PWMController::PWMController(uint8_t pin){
  analogWriteResolution(PWM_BIT_RES);
  analogWriteFrequency(pin, ESC_FREQ);
  initPWM(FAN_STOP);
  pwmPin = pin;
}

void PWMController::initPWM(uint16_t pulseWidth){
  setWidth(pulseWidth);
}

void PWMController::disablePWM(){
  analogWrite(pwmPin, FAN_STOP);
}

void PWMController::setWidth(uint16_t pulseWidth = DEFAULT_PW){

  uint16_t timerThresh = 0;

  //PWM schemes differ (inverted)
  if(pwmPin == FAN_SEL){
    timerThresh = (uint16_t)(TIM_MAX - ((pulseWidth/20000.0)*TIM_MAX));
  } else if (pwmPin == CAM_LASER_SEL){
    timerThresh = (uint16_t)((pulseWidth/20000.0)*TIM_MAX);
  }
  analogWrite(pwmPin, timerThresh);
}
