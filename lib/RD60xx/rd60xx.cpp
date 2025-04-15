#include "rd60xx.h"

RD60xxController::RD60xxController(){
  //Teensy 4.0 pins 0/1 ~ Rx/Tx
  Serial1.begin(115200);
}

void RD60xxController::sendCommand(const byte* command, int length) {

  Serial1.write(command, length);

  Serial1.flush();
}

// Each command registered by the RD60XX PSU will emit an echo. This function will
// pend until the passed command is echoed or return false. Recursive to account 
// for redundant responses
bool RD60xxController::awaitEcho(const byte* command, int length, int attempts){

      byte echo[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

      // 1 second timeout by default
      Serial1.readBytes(echo, MOD_CMD_LEN);
      
      if(memcmp(command, echo, length) == 0) return true;

      if(attempts < 1) return awaitEcho(command, length, ++attempts);

      Serial1.flush();

    return false;
      
}

String RD60xxController::enablePSU(uint16_t deciAmps = DEFAULT_CURRENT){

    uint8_t indx = 0;
    uint8_t failed_attempts = 0;

    setCurrent(deciAmps);

    enable[0] = (char)PSU_ADDR;

    //Calculate 16bit CRC
    uint16_t crc = modCRC16((uint8_t *)enable, 6);
    //Append CRC to instruction
    enable[6] = (char)(0x00FF & crc);
    enable[7] = (char)((0xFF00 & crc) >> 8);

     //Send command, await echo (multi message), resend if not validated
    sendCommand((const byte *)enable, MOD_CMD_LEN);

    while(!awaitEcho((const byte *)enable, MOD_CMD_LEN, 0)){

      failed_attempts++;

      if(failed_attempts > 3) {
        return "Failed to initialize PSU";
      }

      delay(300);

      sendCommand((const byte *)enable, MOD_CMD_LEN);
    }

    psuEnabled = true;

    return "ok";
}

String RD60xxController::disablePSU(){

    uint8_t indx = 0;
    uint8_t failed_attempts = 0;

    disable[0] = (char)PSU_ADDR;

    //Calculate 16bit CRC
    uint16_t crc = modCRC16((uint8_t *)disable, 6);
    //Append CRC to instruction
    disable[6] = (char)(0x00FF & crc);
    disable[7] = (char)((0xFF00 & crc) >> 8);

     //Send command, await echo (multi message), resend if not validated
    sendCommand((const byte *)disable, MOD_CMD_LEN);

    while(!awaitEcho((const byte *)disable, MOD_CMD_LEN, 0)){

      failed_attempts++;

      if(failed_attempts > 3) {
        return "Failed to disable PSU";
      }

      delay(300);

      sendCommand((const byte *)disable, MOD_CMD_LEN);
    }

    psuEnabled = false;

    return "ok";

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