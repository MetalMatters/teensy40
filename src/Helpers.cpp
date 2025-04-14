/*
  Helpers.cpp - Helper functions to be used by OPAL FW on PJRC Teensy 4.x board

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

#include <Arduino.h>
#include "Helpers.h"

/* Nanosecond clock for interpolation 
  TODO: must be called periodically to prevent overflow - use hardware timer */
uint64_t nanos()  // Code by luni @ https://forum.pjrc.com/threads/60493-A-Higher-Resolution-Micros
{
    static uint64_t counter_ns = 0;

    counter_ns += (uint64_t)ARM_DWT_CYCCNT;
    
    ARM_DWT_CYCCNT = 0; 

    return (uint64_t)(counter_ns * (1E9/F_CPU));

}

void printWelcome()
{
  Serial2.print("\n\n\n");
  Serial2.println("********************************************");
  Serial2.println("* Teensy running OpenGalvo OPAL Firmware   *");
  Serial2.println("*  -This is BETA Software                  *");
  Serial2.println("*  -Do not leave any hardware unattended!  *");
  Serial2.println("********************************************");
}


bool isBufferFull() {
  // The buffer is full if the head is right behind the tail
  return ((head + 1) % BUFFER_SIZE) == tail;
}

bool isBufferEmpty() {
  // The buffer is empty if the head is equal to the tail
  return head == tail;
}
