/*

  This is a library for the BH1750FVI Digital Light Sensor breakout board.

  The BH1750 board uses I2C for communication. Two pins are required to
  interface to the device. Configuring the I2C bus is expected to be done
  in user code. The BH1750 library doesn't do this automatically.

  Datasheet: http://www.elechouse.com/elechouse/images/product/Digital%20light%20Sensor/bh1750fvi-e.pdf

  Written by Christopher Laws, March, 2013.

  Porting to SiFive HiFive1 Rev B board by Hoang Ta, March, 2021.

*/

#ifndef BH1750_H
#define BH1750_H

#include <metal/i2c.h>

// Uncomment, to enable debug messages
// #define BH1750_DEBUG

// No active state
#define BH1750_POWER_DOWN 0x00

// Waiting for measurement command
#define BH1750_POWER_ON 0x01

// Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_RESET 0x07

// Default MTreg value
#define BH1750_DEFAULT_MTREG 69

typedef enum
{
    // same as Power Down 
    BH1750_UNCONFIGURED = 0,
    // Measurement at 1 lux resolution. Measurement time is approx 120ms.
    BH1750_CONTINUOUS_HIGH_RES_MODE  = 0x10,
    // Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
    BH1750_CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
    // Measurement at 4 lux resolution. Measurement time is approx 16ms.
    BH1750_CONTINUOUS_LOW_RES_MODE = 0x13,
    // Measurement at 1 lux resolution. Measurement time is approx 120ms.
    BH1750_ONE_TIME_HIGH_RES_MODE = 0x20,
    // Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
    BH1750_ONE_TIME_HIGH_RES_MODE_2 = 0x21,
    // Measurement at 4 lux resolution. Measurement time is approx 16ms.
    BH1750_ONE_TIME_LOW_RES_MODE = 0x23
} Mode;

/* int BH1750_begin(Mode mode = CONTINUOUS_HIGH_RES_MODE, byte addr = 0x23,
           TwoWire* i2c = nullptr); */
int BH1750_begin(Mode mode, unsigned char addr, struct metal_i2c *i2c);
int BH1750_configure(Mode mode);
int BH1750_setMTreg(unsigned char MTreg);
int BH1750_measurementReady(int maxWait);// = false);
float BH1750_readLightLevel();

#endif // BH1750_H
