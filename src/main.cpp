// /*
//   main.cpp - Main projectfile to run OPAL FW on PJRC Teensy 4.x board

//   Part of OpenGalvo - OPAL Firmware

//   Copyright (c) 2020-2021 Daniel Olsson

//   OPAL Firmware is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.

//   OPAL Firmware is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.

//   You should have received a copy of the GNU General Public License
//   along with OPAL Firmware.  If not, see <http://www.gnu.org/licenses/>.
// */

 #include "main.h"

 MotionMGR* motion;
 XY2_100* galvo;


#include <Arduino.h>

int tail; // The actual memory for tail
int head; // The actual memory for head
GCodeCommand buffer[BUFFER_SIZE]; // The actual memory for buffer

typedef struct {
    double x;
    double y;
    double z;
    double i;
    double j;
    double f;
    double p;
    int hasX;
    int hasY;
    int hasZ;
    int hasI;
    int hasJ;
    int hasF;
    int hasP;
} GCodeVals;

// Function prototypes
void processSerialInput();
void storeCommand(const char* cmd);
bool isBufferFull();
bool isBufferEmpty();
void parseGCode(const char* cmd);
bool isValidGCode(const char* cmd);

// Define the serial port and DMA channel
constexpr uint8_t serialPortNumber = 1; // Serial1

// Define the RX buffer
constexpr size_t bufferSize = 64;
volatile uint8_t rxBuffer[bufferSize];

void setup() {

  // Begin serial communication on Serial2 with a baud rate of 115200
  //Serial2.begin(250000);

  // Initialize the buffer
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer[i].valid = false; // No valid commands at start
    buffer[i].X = 0;
    buffer[i].Y = 0;
    buffer[i].code = -1;
  }

  //initialize circular buffer indicies
  head = 0;
  tail = 0;

  //init Galvo Protocol
  galvo = new XY2_100();
  galvo->begin(); //TODO:ADD define "Galvo has SSR" for galvo PSU

  motion = new MotionMGR(buffer);
  motion->begin(galvo);

  ARM_DWT_CYCCNT = 0;

}

void loop() {

  processSerialInput();
  motion->tic();

}

void processSerialInput() {
  static char cmd[MAX_CMD_LENGTH];
  static int cmdIndex = 0;

  while (Serial.available() > 0) {
    char c = Serial.read();
    // Check if we've received a newline, which indicates the end of a command
    if (c == '\n' || c == '\r') {
      cmd[cmdIndex] = '\0'; // Null-terminate the command string
      if (isValidGCode(cmd)) {
        storeCommand(cmd);
      }
      cmdIndex = 0; // Reset the index for the next command
      // Serial.println(cmd);
    } else if (cmdIndex < MAX_CMD_LENGTH - 1) {
      cmd[cmdIndex++] = c; // Add the character to the command string
    }
   }
}

bool isValidGCode(const char* cmd) {
    // Iterate through the string
    while (*cmd != '\0') {
        // Skip leading whitespaces and other characters
        if (*cmd == ' ' || *cmd == '\t' || *cmd == '\r' || *cmd == '\n' || *cmd == ';') {
            cmd++;
            continue;
        }

        // Check if the current position starts with G0 or G1
        if (strncmp(cmd, "G0", 2) == 0 || strncmp(cmd, "G1", 2) == 0) {
            return true;
        }

        if (strncmp(cmd, "G9", 2) == 0) {
            return true;
        }

        // Move to the next character if no match is found
        cmd++;
    }

    // Return false if no G0 or G1 command is found
    return false;
}


GCodeVals parseGCodeCommand(const char *gcode) {
    GCodeVals vals = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0};
    char *next;

    // Find 'X' and convert the following substring to a double
    char *xPtr = strchr(gcode, 'X');
    if (xPtr != NULL) {
        vals.x = strtod(xPtr + 1, &next);
        vals.hasX = (next != xPtr + 1); // Check if a conversion actually happened
    }

    // Find 'Y' and convert the following substring to a double
    char *yPtr = strchr(gcode, 'Y');
    if (yPtr != NULL) {
        vals.y = strtod(yPtr + 1, &next);
        vals.hasY = (next != yPtr + 1); // Check if a conversion actually happened
    }

    // // Find 'Z' and convert the following substring to a double
    // char *zPtr = strchr(gcode, 'Z');
    // if (zPtr != NULL) {
    //     vals.z = strtod(zPtr + 1, &next);
    //     vals.hasZ = (next != zPtr + 1); // Check if a conversion actually happened
    // }

    // // Find 'Y' and convert the following substring to a double
    // char *iPtr = strchr(gcode, 'A');
    // if (iPtr != NULL) {
    //     vals.i = strtod(iPtr + 1, &next);
    //     vals.hasI = (next != iPtr + 1); // Check if a conversion actually happened
    // }

    // // Find 'Y' and convert the following substring to a double
    // char *jPtr = strchr(gcode, 'B');
    // if (jPtr != NULL) {
    //     vals.j = strtod(jPtr + 1, &next);
    //     vals.hasJ = (next != jPtr + 1); // Check if a conversion actually happened
    // }

    // Find 'F' and convert the following substring to a double
    char *fPtr = strchr(gcode, 'F');
    if (fPtr != NULL) {
        vals.f = strtod(fPtr + 1, &next);
        vals.hasF = (next != fPtr + 1); // Check if a conversion actually happened
    }

    // Find 'P' and convert the following substring to a double
    char *pPtr = strchr(gcode, 'P');
    if (pPtr != NULL) {
        vals.p = strtod(pPtr + 1, &next);
        vals.hasP = (next != pPtr + 1); // Check if a conversion actually happened
    }

    return vals;
}

void storeCommand(const char* cmd) {
  if (!isBufferFull()) {
    strncpy(buffer[head].command, cmd, MAX_CMD_LENGTH);
    buffer[head].valid = true;


    if(strncmp(cmd, "G1", 2) == 0){ 
      buffer[head].code = 1; 
    } else if (strncmp(cmd, "G0", 2) == 0){
      buffer[head].code = 0; 
    } else if(strncmp(cmd, "G99", 3) == 0){   //Power enable
      buffer[head].code = 99;
    } else if(strncmp(cmd, "G98", 3) == 0){   //Power disable
      buffer[head].code = 98;
    } else if(strncmp(cmd, "G97", 3) == 0){   //Hopper sequence
      buffer[head].code = 97;
    } else if(strncmp(cmd, "G96", 3) == 0){   //Recoater sequence
      buffer[head].code = 96;
    } else if(strncmp(cmd, "G95", 3) == 0){   //Recoater sequence
      buffer[head].code = 95;
    } else if(strncmp(cmd, "G94", 3) == 0){   //Recoater sequence
      buffer[head].code = 94;
    }
    
    // else if(strncmp(cmd, "G0", 2) == 0){ 
    //   buffer[head].code = 0; 
    // } else {
    //    buffer[head].code = 1; 
    // }

    GCodeVals values = parseGCodeCommand(cmd);

    buffer[head].X = values.hasX ? ((values.x + X_OFFSET)*X_CAL): -1;
    buffer[head].Y = values.hasY ? ((values.y + Y_OFFSET)*Y_CAL): -1;
    buffer[head].Z = values.hasZ ? values.z : -1;
    buffer[head].I = values.hasI ? values.i : -1;
    buffer[head].J = values.hasJ ? values.j : -1;
    buffer[head].F = values.hasF ? values.f : -1;
    buffer[head].P = values.hasP ? values.p : -1;

    head = (head + 1) % BUFFER_SIZE; // Move to the next slot in the buffer
  
  } else {
    // Buffer is full, handle this condition (e.g., by discarding the oldest command)
    // For now, we'll just print an error message
    Serial.println("Buffer is full, command discarded.");
  }
}

void setGalvoPosition(double x, double y)
{
  int tmp_x, tmp_y;
  if(AXIS_INVERSE_X)
    tmp_x = (int)map(x, 0.0,X_MAX_POS_MM, 65535,0)+0.5;
  else
    tmp_x = (int)map(x, 0.0,X_MAX_POS_MM, 0,65535)+0.5;

  if(AXIS_INVERSE_Y)
    tmp_y = (int)map(y, 0.0,Y_MAX_POS_MM, 65535,0)+0.5;
  else
    tmp_y = (int)map(y, 0.0,Y_MAX_POS_MM, 0,65535)+0.5;

  galvo->setPos((uint16_t)tmp_x, (uint16_t)tmp_y);
}

