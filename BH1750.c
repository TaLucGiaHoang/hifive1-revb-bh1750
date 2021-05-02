/*

  This is a library for the BH1750FVI Digital Light Sensor breakout board.

  The BH1750 board uses I2C for communication. Two pins are required to
  interface to the device. Configuring the I2C bus is expected to be done
  in user code. The BH1750 library doesn't do this automatically.

  Datasheet: http://www.elechouse.com/elechouse/images/product/Digital%20light%20Sensor/bh1750fvi-e.pdf

  Written by Christopher Laws, March, 2013.

  Porting to SiFive HiFive1 Rev B board by Hoang Ta on April, 2021.

*/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <metal/machine.h>
#include <metal/i2c.h>
#include "BH1750.h"

//#define BH1750_DEBUG
extern unsigned long millis(void);
extern void delay(uint32_t miliseconds);
#define _delay_ms(ms) delay(ms)

unsigned int BH1750_I2CADDR = 0x23;  // default is 0x23
unsigned char BH1750_MTreg = (unsigned char)BH1750_DEFAULT_MTREG;
// Correction factor used to calculate lux. Typical value is 1.2 but can
// range from 0.96 to 1.44. See the data sheet (p.2, Measurement Accuracy)
// for more information.
const float BH1750_CONV_FACTOR = 1.2;
Mode BH1750_MODE = BH1750_UNCONFIGURED;  // default is BH1750_CONTINUOUS_HIGH_RES_MODE
struct metal_i2c *I2C;
unsigned long long lastReadTimestamp;

/**
 * Configure sensor
 * @param mode Measurement mode
 * @param addr Address of the sensor (0x23 or 0x5C, see datasheet)
 * @param i2c Object pointer connected to I2C bus
 * return 1 (true) or 0 (false)
 */
int BH1750_begin(Mode mode, unsigned char addr, struct metal_i2c *i2c) {

  // I2C is expected to be initialized outside this library
  // But, allows a different address and Metal I2C struct pointer to be used
  if(i2c) {
    I2C = i2c;
  }
  if(addr) {
    BH1750_I2CADDR = addr;
  }

  // Configure sensor in specified mode and set default MTreg
  return (BH1750_configure(mode) && BH1750_setMTreg(BH1750_DEFAULT_MTREG));
}


/**
 * Configure BH1750 with specified mode
 * @param mode Measurement mode
 */
int BH1750_configure(Mode mode) {
  unsigned char cmd[1];
  // default transmission result to a value out of normal range
  unsigned char ack = 5;

  // Check measurement mode is valid
  switch (mode) {
    case BH1750_CONTINUOUS_HIGH_RES_MODE:
    case BH1750_CONTINUOUS_HIGH_RES_MODE_2:
    case BH1750_CONTINUOUS_LOW_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE_2:
    case BH1750_ONE_TIME_LOW_RES_MODE:
      // Send mode to sensor
      metal_i2c_transfer(I2C, BH1750_I2CADDR, (unsigned char*)&mode, 1, &ack, 1);

      // Wait a few moments to wake up
      _delay_ms(10);
      break;

    default:
      // Invalid measurement mode
      printf("[BH1750] ERROR: Invalid mode\r\n");
      break;

  }

  // Check result code
  switch (ack) {
    case 0:
      BH1750_MODE = mode;
      lastReadTimestamp = millis();
      return true;
    case 1: // too long for transmit buffer
      printf("[BH1750] ERROR: too long for transmit buffer\r\n");
      break;
    case 2: // received NACK on transmit of address
      printf("[BH1750] ERROR: received NACK on transmit of address\r\n");
      break;
    case 3: // received NACK on transmit of data
      printf("[BH1750] ERROR: received NACK on transmit of data\r\n");
      break;
    case 4: // other error
      printf("[BH1750] ERROR: other error\r\n");
      break;
    default:
      printf("[BH1750] ERROR: undefined error\r\n");
      break;
  }

  return false;

}

/**
 * Configure BH1750 MTreg value
 * MT reg = Measurement Time register
 * @param MTreg a value between 32 and 254. Default: 69
 * @return bool true if MTReg successful set
 * 		false if MTreg not changed or parameter out of range
 */
int BH1750_setMTreg(unsigned char MTreg) {
  //Bug: lowest value seems to be 32!
  if (MTreg <= 31 || MTreg > 254) {
    printf("[BH1750] ERROR: MTreg out of range\r\n");
    return false;
  }
  unsigned char byte;
  unsigned char ack = 5;
  // Send MTreg and the current mode to the sensor
  //   High bit: 01000_MT[7,6,5]
  //    Low bit: 011_MT[4,3,2,1,0]
  byte = (0b01000 << 3) | (MTreg >> 5);
  metal_i2c_transfer(I2C, BH1750_I2CADDR, &byte, 1, &ack, 1);

  byte = (0b011 << 5 )  | (MTreg & 0b11111);
  metal_i2c_transfer(I2C, BH1750_I2CADDR, &byte, 1, &ack, 1);

  byte = BH1750_MODE;
  metal_i2c_transfer(I2C, BH1750_I2CADDR, &byte, 1, &ack, 1);

  // Wait a few moments to wake up
  _delay_ms(10);

  // Check result code
  switch (ack) {
    case 0:
      BH1750_MTreg = MTreg;
      return true;
    case 1: // too long for transmit buffer
      printf("[BH1750] ERROR: too long for transmit buffer\r\n");
      break;
    case 2: // received NACK on transmit of address
      printf("[BH1750] ERROR: received NACK on transmit of address\r\n");
      break;
    case 3: // received NACK on transmit of data
      printf("[BH1750] ERROR: received NACK on transmit of data\r\n");
      break;
    case 4: // other error
      printf("[BH1750] ERROR: other error\r\n");
      break;
    default:
      printf("[BH1750] ERROR: undefined error\r\n");
      break;
  }

  return false;
}

/**
 * Checks whether enough time has gone to read a new value
 * @param maxWait a boolean if to wait for typical or maximum delay
 *                1 (true), 0 (false), default value is 0
 * @return a boolean if a new measurement is possible
 * 
 */
int BH1750_measurementReady(int maxWait) {
  unsigned long delaytime = 0;
  switch (BH1750_MODE) {
    case BH1750_CONTINUOUS_HIGH_RES_MODE:
    case BH1750_CONTINUOUS_HIGH_RES_MODE_2:
    case BH1750_ONE_TIME_HIGH_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE_2:
      delaytime = maxWait ? (180 * BH1750_MTreg/(unsigned char)BH1750_DEFAULT_MTREG) : (120 * BH1750_MTreg/(unsigned char)BH1750_DEFAULT_MTREG);
      break;
    case BH1750_CONTINUOUS_LOW_RES_MODE:
    case BH1750_ONE_TIME_LOW_RES_MODE:
      // Send mode to sensor
      delaytime = maxWait ? (24 * BH1750_MTreg/(unsigned char)BH1750_DEFAULT_MTREG) : (16 * BH1750_MTreg/(unsigned char)BH1750_DEFAULT_MTREG);
      break;
    default:
      break;
  }
  // Wait for new measurement to be possible.
  // Measurements have a maximum measurement time and a typical measurement
  // time. The maxWait argument determines which measurement wait time is
  // used when a one-time mode is being used. The typical (shorter)
  // measurement time is used by default and if maxWait is set to True then
  // the maximum measurement time will be used. See data sheet pages 2, 5
  // and 7 for more details.
  unsigned long long currentTimestamp = millis();
    if (currentTimestamp - lastReadTimestamp >= delaytime) {
      return true;
    }
    else
      return false;
}

/**
 * Read light level from sensor
 * The return value range differs if the MTreg value is changed. The global
 * maximum value is noted in the square brackets.
 * @return Light level in lux (0.0 ~ 54612,5 [117758,203])
 * 	   -1 : no valid return value
 * 	   -2 : sensor not configured
 */
float BH1750_readLightLevel() {

  if (BH1750_MODE == BH1750_UNCONFIGURED) {
    printf("[BH1750] Device is not configured!\r\n");
    return -2.0;
  }

  // Measurement result will be stored here
  float level = -1.0;

  // Read two bytes from the sensor, which are low and high parts of the sensor
  // value
  unsigned char tmp[2] = {0, 0};
  metal_i2c_read(I2C, BH1750_I2CADDR, 2, tmp, METAL_I2C_STOP_ENABLE);
  level = (float)((tmp[0] << 8) | tmp[1]);

  lastReadTimestamp = millis();

  if (level != -1.0) {
    // Print raw value if debug enabled
    #ifdef BH1750_DEBUG
    printf("[BH1750] Raw value: %f\r\n", level);
    #endif

    if (BH1750_MTreg != BH1750_DEFAULT_MTREG) {
      level *= (float)((unsigned char)BH1750_DEFAULT_MTREG/(float)BH1750_MTreg);
      // Print MTreg factor if debug enabled
      #ifdef BH1750_DEBUG
      printf("[BH1750] MTreg factor: %f\r\n", (float)((unsigned char)BH1750_DEFAULT_MTREG/(float)BH1750_MTreg));
      #endif
    }
    if (BH1750_MODE == BH1750_ONE_TIME_HIGH_RES_MODE_2 || BH1750_MODE == BH1750_CONTINUOUS_HIGH_RES_MODE_2) {
      level /= 2;
    }
    // Convert raw value to lux
    level /= BH1750_CONV_FACTOR;

    // Print converted value if debug enabled
    #ifdef BH1750_DEBUG
    printf("[BH1750] Converted float value: %f\r\n", level);
    #endif
  }

  return level;
}
