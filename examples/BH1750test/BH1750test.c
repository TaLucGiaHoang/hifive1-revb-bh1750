/*
  BH1750test.c

  Created on: April 30, 2021
      Author: Hoang Ta

  Example of BH1750 library usage.

  This example initializes the BH1750 object using the default high resolution
  continuous mode and then makes a light level reading every second.

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
  BH1750_begin(BH1750_CONTINUOUS_HIGH_RES_MODE, 0x23, bh1750_i2c);
  printf("Test BH1750 Test begin\r\n");

  while(1) {
    float lux = BH1750_readLightLevel();
    printf("Light: %f lx\r\n", lux);
    delay(1000);
  }

  return 0;
}
