/*
  BH1750two_i2c.c

  Created on: April 30, 2021
      Author: Hoang Ta

  Example of BH1750 library usage.

  This example initializes two BH1750 objects using the default high resolution
  one shot mode and then makes a light level reading every second.

  Connection:

    BH1750 A:
      VCC -> 3V3 or 5V
      GND -> GND
      SCL -> SCL
      SDA -> SDA
      ADD -> (not connected) or GND

    BH1750 B:
      VCC -> 3V3 or 5V
      GND -> GND
      SCL -> SCL
      SDA -> SDA
      ADD -> VCC

  ADD pin is used to set sensor I2C address. If it has voltage greater or equal to
  0.7VCC voltage (e.g. you've connected it to VCC) the sensor address will be
  0x5C. In other case (if ADD voltage less than 0.7 * VCC) the sensor address will
  be 0x23 (by default).

*/

#include <stdbool.h>
#include <stdio.h>
#include <metal/i2c.h>
#include "BH1750.h"
struct metal_i2c *i2c;
struct BH1750_sensor *bh1750_a;
struct BH1750_sensor *bh1750_b;
static int error_counter_1_a = 0;
static int error_counter_2_a = 0;
static int error_counter_1_b = 0;
static int error_counter_2_b = 0;

extern void delay(uint32_t miliseconds);

int main() {
  asm (".global _printf_float");  // add this line for printf and scanf be able to support float type

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  i2c = metal_i2c_get_device(0);
  if (i2c == NULL) {
    printf("I2C not available\r\n");
    return -1;
  }
  metal_i2c_init(i2c, 100000, METAL_I2C_MASTER); // configure to 100000Hz, master mode

  bh1750_a = BH1750_begin(BH1750_CONTINUOUS_HIGH_RES_MODE, 0x23, i2c, 0);  // sensor A, address 0x23
  bh1750_b = BH1750_begin(BH1750_CONTINUOUS_HIGH_RES_MODE, 0x5C, i2c, 0);  // sensor B, address 0x5C
  printf("BH1750 Test begin\r\n");

  while(1) {
    float light_level_a, light_level_b;
    if (BH1750_measurementReady(bh1750_a, 0)) {
      light_level_a = BH1750_readLightLevel(bh1750_a);
    }

    if (BH1750_measurementReady(bh1750_b, 0)) {
      light_level_b = BH1750_readLightLevel(bh1750_b);
    }

    if (light_level_a == -1.0) {
      error_counter_1_a++;
    }
    if (light_level_a == -2.0) {
      error_counter_2_a++;
    }
    if (light_level_b == -1.0) {
      error_counter_1_b++;
    }
    if (light_level_b == -2.0) {
      error_counter_2_b++;
    }

    printf("A: %f lux %d:%d | B: %f lux %d:%d\r\n", light_level_a,
                  error_counter_1_a, error_counter_2_a, light_level_b,
                  error_counter_1_b, error_counter_2_b);
    delay(1000);
  }
  return 0;
}
