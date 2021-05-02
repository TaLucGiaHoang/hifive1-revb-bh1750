/*
  BH1750advanced.c

  Created on: April 30, 2021
      Author: Hoang Ta

  Advanced BH1750 library usage example

  This example has some comments about advanced usage features.

    BH1750 has six different measurement modes. They are divided in two groups;
    continuous and one-time measurements. In continuous mode, sensor continuously
    measures lightness value. In one-time mode the sensor makes only one
    measurement and then goes into Power Down mode.

    Each mode, has three different precisions:

      - Low Resolution Mode - (4 lx precision, 16ms measurement time)
      - High Resolution Mode - (1 lx precision, 120ms measurement time)
      - High Resolution Mode 2 - (0.5 lx precision, 120ms measurement time)

    By default, the library uses Continuous High Resolution Mode, but you can
    set any other mode, by passing it to BH1750_begin() or BH1750_configure()
    functions.

    [!] Remember, if you use One-Time mode, your sensor will go to Power Down
    mode each time, when it completes a measurement and you've read it.

    Full mode list:

      BH1750_CONTINUOUS_LOW_RES_MODE
      BH1750_CONTINUOUS_HIGH_RES_MODE (default)
      BH1750_CONTINUOUS_HIGH_RES_MODE_2

      BH1750_ONE_TIME_LOW_RES_MODE
      BH1750_ONE_TIME_HIGH_RES_MODE
      BH1750_ONE_TIME_HIGH_RES_MODE_2

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

  // begin returns a boolean that can be used to detect setup problems.
  if(BH1750_begin(BH1750_CONTINUOUS_HIGH_RES_MODE, 0x23, bh1750_i2c) == true)
  {
    printf("BH1750 Advanced begin\r\n");
  } else {
    printf("Error initializing BH1750\r\n");
  }

  while(1) {
    if(BH1750_measurementReady(0)) {
      float lux = BH1750_readLightLevel();
      printf("Light: %f lx\r\n", lux);
    }

    delay(1000); // delay for next measurement
  }

  return 0;
}
