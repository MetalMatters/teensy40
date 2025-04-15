/*
  MotionMGR.h - driver code for Synrad 48 Series laser on PJRC Teensy 4.x board

  Part of OpenGalvo - OPAL Firmware

  Copyright (c) 2020-2021 Daniel Olsson

  OPAL Firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPAL Firmware is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPAL Firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifdef __AVR__
#error "Sorry, this only works on 32 bit Teensy boards.  AVR isn't supported."
#endif

#if TEENSYDUINO < 121
#error "Minimum PJRC Teensyduino version 1.21 is required"
#endif

#ifndef MOTIONMGR_h
#define MOTIONMGR_h

#include <rd60xx.h>
#include <pwmController.h>
#include "Helpers.h"
#include "configuration.h"
#include "main.h"
#include "MotorController.h"

// Fan controller pin
#define FAN_PIN 7
// Welding camera laser pin
#define CAMERA_LASER_PIN 8

// Piston velocity
#define PISTON_FREQ 300
// Recoater velocity
#define RECOATER_FREQ 1800

#define BUILD_STEPS 686  // 1um step size (2mm lead) 886 ~ 45um   ####1078
#define POWDER_STEPS 985 // 72um step size 815 ~ 90um
#define RECOATER_STEPS 3400


enum MotionStatus {IDLE, INTERPOLATING};

class MotionMGR
{
public:
  MotionMGR(GCodeCommand *buf);
  void begin(XY2_100 *galvo);
  void tic();
  MotionStatus getStatus();

private:
  MotionStatus _status;
  RD60xxController* psu;
  MotorController* motors;
  PWMController*  fan;
  PWMController*  laser5W;
  void processGcode(GCodeCommand* code);
  bool processGcodes();
  void awaitMovement();
  void interpolateMove();
  double calculateMoveLengthNanos(double xdist, double ydist, double moveVelocity, uint64_t* result);
  void setVal(double* varToSet, double valToSet);
  void setValG91(double* varToSet, double valToAdd, double base);
  void setXY(GCodeCommand* code);
  
  GCodeCommand *buffer;

  bool isMoveFirstInterpolation = true;

  uint64_t CURRENT_STARTNANOS = 0;
  uint64_t CURRENT_ENDNANOS = 0;

  uint64_t _NOW = 0;
  int CURRENT_CODE = 0;
  double CURRENT_FROM_X = 0;
  double CURRENT_FROM_Y = 0;
  double CURRENT_FROM_Z = 0;
  double CURRENT_DISTANCE_X = 0;
  double CURRENT_DISTANCE_Y = 0;
  double CURRENT_DISTANCE_Z = 0;
  double CURRENT_TO_X = 0;
  double CURRENT_TO_Y = 0;
  double CURRENT_TO_Z = 0;
  double CURRENT_CMD_X = 0;
  double CURRENT_CMD_Y = 0;
  double CURRENT_CMD_Z = 0;
  double CURRENT_I = 0;
  double CURRENT_J = 0;
  double CURRENT_F = DEFAULT_FEEDRATE;
  double CURRENT_S = 0;
  uint64_t CURRENT_DURATION = 0;
  bool CURRENT_ABSOLUTE = true;
  bool CURRENT_LASERENABLED = false;
  bool LASER_CHANGED = false;

  // FPGA interface pins
  const static int active_flag = 15;
  const static int reset_flag = 11;  //21 - P100
  const static int layer_complete_flag = 10;  //16 - P106
  const static int test_flag = 18; //P104

};
#endif