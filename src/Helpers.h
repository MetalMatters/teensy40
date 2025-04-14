/*
  Helpers.h - Helper functions to be used by OPAL FW on PJRC Teensy 4.x board

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

#ifndef HELPERS_H

#define MAX_VAL 2147483630

//Mcodes block Marlin buffer to generate keepalive feedback, pending gcode movements
#define M0_PAUSE "M0"
#define M108_RESUME "M108"

uint64_t nanos();

// Define the maximum length of a G-code command and the buffer size
#define MAX_CMD_LENGTH  50
#define BUFFER_SIZE     25

  // Define a structure to hold a G-code command
struct GCodeCommand {
  char command[MAX_CMD_LENGTH];
  bool valid; // Indicates if the command is valid and ready to be processed
  double X;
  double Y;
  double Z;
  double I;
  double J;
  double F;
  double P;
  int code;
};

#ifndef GLOBALS_H
#define GLOBALS_H

extern int tail;
extern int head;
extern GCodeCommand buffer[];

#endif // GLOBALS_H

struct coordinate {
  double x = MAX_VAL;
  double y = MAX_VAL;
  double z = MAX_VAL;
};

bool isBufferFull();

bool isBufferEmpty();

void printDouble( double val, unsigned int precision);

#define HELPERS_H
#endif
