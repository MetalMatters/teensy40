#include "rd60xx.h"

RD60xxController::RD60xxController(){
  //Teensy 4.0 pins 0/1 ~ Rx/Tx
  Serial1.begin(115200);
}

void RD60xxController::enablePSU(uint16_t deciAmps = DEFAULT_CURRENT){

    uint8_t indx = 0;

    setCurrent(deciAmps);

    enable[0] = (char)PSU_ADDR;

    //Calculate 16bit CRC
    uint16_t crc = modCRC16((uint8_t *)enable, 6);
    //Append CRC to instruction
    enable[6] = (char)(0x00FF & crc);
    enable[7] = (char)((0xFF00 & crc) >> 8);

    while(indx < MOD_CMD_LEN) Serial1.write(enable[indx++]);

    psuEnabled = true;

    delay(200);

}

void RD60xxController::disablePSU(){

    uint8_t indx = 0;

    disable[0] = (char)PSU_ADDR;

    //Calculate 16bit CRC
    uint16_t crc = modCRC16((uint8_t *)disable, 6);
    //Append CRC to instruction
    disable[6] = (char)(0x00FF & crc);
    disable[7] = (char)((0xFF00 & crc) >> 8);

    while(indx < MOD_CMD_LEN) Serial1.write(disable[indx++]);

    psuEnabled = false;

    delay(200);

}

void RD60xxController::setCurrent(uint16_t deciAmps){

      uint8_t indx = 0;
        //Set slave address
      i_set[0] = (char)PSU_ADDR;

      //Populate instruction with parsed current level (milliamps x 10)
      i_set[4] = (char)((0xFF00 & deciAmps) >> 8);
      i_set[5] = (char)(0x00FF & deciAmps);
      //Calculate 16bit CRC
      uint16_t crc = modCRC16((uint8_t *)i_set, 6);
      //Append CRC to instruction
      i_set[6] = (char)(0x00FF & crc);
      i_set[7] = (char)((0xFF00 & crc) >> 8);

      while(indx < MOD_CMD_LEN) Serial1.write(i_set[indx++]);

      delay(200);

}

void RD60xxController::setVoltage(uint16_t deciVolts = DEFAULT_VOLTAGE){

      uint8_t indx = 0;
      //Set slave address
      v_set[0] = (char)PSU_ADDR;

      //Populate instruction with parsed current level (milliamps x 10)
      v_set[4] = (char)((0xFF00 & deciVolts) >> 8);
      v_set[5] = (char)(0x00FF & deciVolts);
      //Calculate 16bit CRC
      uint16_t crc = modCRC16((uint8_t *)v_set, 6);
      //Append CRC to instruction
      v_set[6] = (char)(0x00FF & crc);
      v_set[7] = (char)((0xFF00 & crc) >> 8);

      while(indx < MOD_CMD_LEN) Serial1.write(v_set[indx++]);

      delay(200);
}

uint16_t RD60xxController::modCRC16(uint8_t *buf, int len)
{

  uint16_t crc = 0xFFFF;
  
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc
  
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }

  return crc;

}