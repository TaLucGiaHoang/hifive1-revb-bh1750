/*
  BH1750autoadjust.c

  Created on: April 30, 2021
      Author: Hoang Ta

  Example of BH1750 library usage.

  This example initialises the BH1750 object using the default high resolution
  one shot mode and then makes a light level reading every five seconds.

  After the measurement the MTreg value is changed according to the result:
  lux > 40000 ==> MTreg =  32
  lux < 40000 ==> MTreg =  69  (default)
  lux <    10 ==> MTreg = 138
  Remember to test your specific sensor! Maybe the MTreg value range from 32
  up to 254 is not applicable to your unit.

  Connection:

    VCC -> 3V3 or 5V
    GND -> GND
    SCL -> SCL
    SDA -> SDA
    ADD -> (not connected) or GND

  ADD pin is used to set sensor I2C address. If it has voltage greater or equal to
  0.7VCC voltage (e.g. you've connected it to VCC) the sensor address will be
  0x5C. In other case (if ADD voltage less than 0.7 * VCC) the sensor address will
  be 0x23 (by default).

*/

#include <stdbool.h>
#include <stdio.h>
#include <metal/i2c.h>
#include "BH1750.h"
struct metal_i2c *bh1750_i2c;

extern void delay(uint32_t miliseconds);

int main() {
  asm (".global _printf_float");  // add this line for printf and scanf be able to support float type

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  bh1750_i2c = metal_i2c_get_device(0);
  if (bh1750_i2c == NULL) {
    printf("I2C not available \n");
    return -1;
  }
  metal_i2c_init(bh1750_i2c, 100000, METAL_I2C_MASTER); // configure to 100000Hz, master mode

  BH1750_begin(BH1750_ONE_TIME_HIGH_RES_MODE, 0x23, bh1750_i2c);
  printf("BH1750 Test begin\r\n");

  while(1) {
    //we use here the maxWait option due fail save
    if(BH1750_measurementReady(1)) {
      float lux = BH1750_readLightLevel();
      printf("Light: %f lx\r\n", lux);

      if(lux < 0) {
        printf("Error condition detected\r\n");
      }
      else {
        if(lux > 40000.0) {
          // reduce measurement time - needed in direct sun light
          if(BH1750_setMTreg(32)) {
            printf("Setting MTReg to low value for high light environment\r\n");
          }
          else {
            printf("Error setting MTReg to low value for high light environment\r\n");
          }
        }
        else {
          if(lux > 10.0) {
            // typical light environment
            if(BH1750_setMTreg(69)) {
              printf("Setting MTReg to default value for normal light environment\r\n");
            }
            else {
              printf("Error setting MTReg to default value for normal light environment\r\n");
            }
          }
          else {
            if(lux <= 10.0) {
              //very low light environment
              if (BH1750_setMTreg(138)) {
                printf("Setting MTReg to high value for low light environment\r\n");
              }
              else {
                printf("Error setting MTReg to high value for low light environment\r\n");
              }
            }
          }
        }
      }
      printf("--------------------------------------\r\n");
    }
    delay(5000);
  }

  return 0;
}
