#pragma once

#ifndef RD60xx_h
#define RD60xx_h

#include <Arduino.h>

#define PSU_ADDR 0x01
#define MOD_CMD_LEN 8

#define DEFAULT_VOLTAGE 2450 //decivolts
#define DEFAULT_CURRENT 500 //deciamps

 class RD60xxController {


  public:
    RD60xxController();
    void enablePSU(uint16_t deciAmps = DEFAULT_CURRENT);
    void disablePSU();
    void setCurrent(uint16_t deciAmps);
    void setVoltage(uint16_t deciVolts = DEFAULT_VOLTAGE);

  private:
    bool psuEnabled = false;
    uint32_t currentAmps = DEFAULT_CURRENT;
    uint32_t currentVoltage = DEFAULT_VOLTAGE;

   /* Modbus commands:
    * Slave Addr - Func Code - Data Address -  Data  -   CRC
    *   1 byte       1 byte      2 bytes     2 bytes   2 bytes
    */
    char enable[MOD_CMD_LEN] = {0x00, 0x06, 0x00, 0x12, 0x00, 0x01, 0xE8, 0x0F};
    char disable[MOD_CMD_LEN] = {0x00, 0x06, 0x00, 0x12, 0x00, 0x00, 0x29, 0xCF};
    char v_set[MOD_CMD_LEN] = {0x00, 0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};
    char i_set[MOD_CMD_LEN] = {0x00, 0x06, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00};

    uint16_t modCRC16(uint8_t *buf, int len);

};

#endif